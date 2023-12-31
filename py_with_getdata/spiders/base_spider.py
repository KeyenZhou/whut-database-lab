import traceback
from datetime import datetime, timedelta
from bs4 import BeautifulSoup
import requests
import time
import calendar
import pymysql as mysql
import re
from selenium.common import TimeoutException
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.select import Select
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from utils.helpers import insert_sql
from utils.helpers import Wrapper
from config import web_config
from config import url_config
from utils.helpers import query_appid
from utils.helpers import find_appid_in_game_library
from utils.helpers import parse_date
import logging
from utils.helpers import extract_numbers_with_comma_and_dot
from utils.helpers import find_appid_in_game

logging.basicConfig(filename=r'logs/spider.log', level=logging.ERROR)


def get_game_library(db: mysql.connections):
    for i in range(1, url_config['game_library_num'] + 1):
        state_code = get_game_page(db, i)
        if state_code != 200:
            print(f"page {i} 连接超时: {state_code}")
            time.sleep(60)
            get_game_page(db, i)


def get_game_page(db: mysql.connections.Connection, index: int) -> int:
    sql = '''
        INSERT INTO game_library (AppID, game_name) VALUES (%s,%s);
    '''
    url = url_config['game_library_url'].format(index)
    response = requests.get(url=url, **web_config)
    if response.status_code == 200:
        response.encoding = 'utf-8'
        soup = BeautifulSoup(response.text, 'lxml')
        table = soup.find('div', id='search_resultsRows')
        for App in table.find_all('a', recursive=False):
            AppID = App.get('data-ds-appid')
            AppName = App.find('span', class_='title')
            if AppID and AppName:
                try:
                    insert_sql(db, sql, (int(AppID.strip()), AppName.text.strip()))
                except Exception as e:
                    print(f"{AppID}, {AppName} 无法被解析")
                    print(f"Error: {e}")
        return 200
    else:
        return 0


# 爬取每周排行榜
def get_week_rank(db: mysql.connections.Connection):
    # week_rank
    cursor = db.cursor()
    sql = '''
        INSERT INTO week_rank (Date, AppID,`rank`) VALUES (%s,%s,%s);
    '''
    start_date = datetime.strptime(url_config['start_date'], '%Y-%m-%d')
    end_date = datetime.strptime(url_config['end_date'], '%Y-%m-%d')
    interval_days = 7

    date_list = []

    current_date = start_date
    while current_date.date() <= end_date.date():
        date_list.append(current_date.date())
        current_date += timedelta(days=interval_days)
    try:
        for date in date_list:
            formatted_date = date.strftime('%Y-%m-%d')
            url = url_config['week_rank_url'].format(formatted_date)
            # 初始化 WebDriver
            driver = webdriver.Chrome()

            # 打开网页
            driver.get(url)

            # 休息
            time.sleep(5)

            # 往下滑动
            driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")
            # 等待页面加载完成，可以根据需要选择适当的等待条件

            try:
                # 等待table加载完成
                WebDriverWait(driver, 60).until(EC.presence_of_element_located((By.TAG_NAME, 'table')))
            except TimeoutException:
                print("Timed out waiting for page to load")

            # 确保
            time.sleep(5)

            # 获取页面源码
            page_source = driver.page_source

            # 关闭 WebDriver
            driver.quit()
            soup = BeautifulSoup(page_source, 'lxml')
            table = soup.find('table', class_='weeklytopsellers_ChartTable_3arZn')
            tbody = table.find('tbody')
            trs = tbody.find_all('tr')
            for tr in trs:
                a = tr.find('a', class_='weeklytopsellers_TopChartItem_2C5PJ')
                if a:
                    match = re.search(r'/app/(\d+)', a.get('href'))
                    if match:
                        app_info = int(match.group(1))
                        print("App Info:", app_info)
                    else:
                        print(f"No match found.{a}")
                        continue
                else:
                    continue
                td = tr.find('td', class_='weeklytopsellers_RankCell_34h48')
                if td:
                    rank = td.text.strip()
                else:
                    continue
                insert_sql(db, sql, (formatted_date, app_info, rank))
                game = tr.find('div', class_='weeklytopsellers_GameName_1n_4-')
                if game:
                    game_name = game.text.strip()
                    if query_appid(db, app_info) is False:
                        sql_library = '''
                                        INSERT INTO game_library (AppID, game_name) VALUES (%s,%s);
                                    '''
                        insert_sql(db, sql_library, (app_info, game_name))
                else:
                    print(f"AppID {app_info} 无法得到游戏名")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        cursor.close()


# 爬取月排行
def get_month_rank(db: mysql.connections.Connection):
    cursor = db.cursor()
    sql = '''
        INSERT INTO month_rank (Date, AppID) VALUES (%s,%s);
    '''
    try:
        # 获取一年中所有月份的名称
        months = calendar.month_name[1:12]
        for month in months:
            url = url_config['month_rank_url'].format(month)
            driver = webdriver.Chrome()
            driver.get(url)

            time.sleep(5)

            driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")
            try:
                # 等待table加载完成
                WebDriverWait(driver, 60).until(
                    EC.presence_of_element_located((By.CSS_SELECTOR,
                                                    '#SaleSection_59496 > div > div.Panel.Focusable > div:nth-child('
                                                    '20) > div > div > div > div > '
                                                    'div.salepreviewwidgets_StoreSaleWidgetLeft_w77ms > div > a > div '
                                                    '> '
                                                    'div.salepreviewwidgets_HeroCapsuleImageContainer_1ail_'
                                                    '.HeroCapsuleImageContainer > img')))

            except TimeoutException:
                print("Timed out waiting for page to load")

            time.sleep(5)
            # 获取页面源码
            page_source = driver.page_source
            driver.quit()
            soup = BeautifulSoup(page_source, 'lxml')
            table = soup.select_one('#SaleSection_59496 > div > div.Panel.Focusable')
            for div in table.find_all('div', recursive=False):
                a = div.select_one(
                    'div > div > div > div > '
                    'div.salepreviewwidgets_StoreSaleWidgetLeft_w77ms > div > a > div '
                    '> '
                    'div.salepreviewwidgets_HeroCapsuleImageContainer_1ail_'
                    '.HeroCapsuleImageContainer > img')

                if a:
                    match = re.search(r'/apps/(\d+)', a.get('src'))
                    if match:
                        app_info = int(match.group(1))
                        print("App Info:", app_info)
                    else:
                        print(f"No match found.{a}")
                        continue
                    month_number = list(calendar.month_name).index(month)
                    current_year = datetime.now().year
                    formatted_date = f"{current_year}-{month_number:02d}-01"
                    insert_sql(db, sql, (formatted_date, app_info))
                    game_name = a.get('alt').strip()
                    if query_appid(db, app_info) is False:
                        sql_library = '''
                                        INSERT INTO game_library (AppID, game_name) VALUES (%s,%s);
                                    '''
                        insert_sql(db, sql_library, (app_info, game_name))
    except Exception as e:
        print(f"Error: {e}")
    finally:
        cursor.close()


# 爬取年排行
def get_year_rank(db: mysql.connections.Connection):
    cursor = db.cursor()
    date = '2022-01-01'
    sql = '''
        INSERT INTO year_rank (Date, AppID,category) VALUES (%s,%s,%s);
    '''
    ranks = {
        '铂金': 'SaleSection_18829',
        '黄金': 'SaleSection_19046',
        '白银': 'SaleSection_48239',
        '青铜': 'SaleSection_89135'
    }
    url = url_config['year_rank_url']
    try:
        driver = webdriver.Chrome()
        driver.get(url)
        time.sleep(5)
        driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")
        time.sleep(5)
        driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")
        time.sleep(5)
        driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")

        time.sleep(20)
        driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")
        time.sleep(5)
        page_source = driver.page_source
        driver.close()
        soup = BeautifulSoup(page_source, 'lxml')
        for (k, v) in ranks.items():
            div = soup.find('div', id=v)
            # print(div)
            imgs = div.find_all('img', class_='salepreviewwidgets_CapsuleImage_cODQh')
            for img in imgs:
                match = re.search(r'/apps/(\d+)', img.get('src'))
                if match:
                    app_info = int(match.group(1))
                    print("App Info:", app_info)
                else:
                    print(f"No match found.{img}")
                    continue
                insert_sql(db, sql, (date, app_info, k))
                game_name = img.get('alt').strip()
                if query_appid(db, app_info) is False:
                    sql_library = '''
                                    INSERT INTO game_library (AppID, game_name) VALUES (%s,%s);
                                '''
                    insert_sql(db, sql_library, (app_info, game_name))
    except Exception as e:
        print(f"Error: {e}")
    finally:
        cursor.close()


def find_dev(db: mysql.connections.Connection, name: str, url: str) -> int:
    try:
        dev_sql = '''
            select dev_pub_id,developer_name from dev_pub_er;
        '''
        cursor = db.cursor()
        cursor.execute(dev_sql)
        results = cursor.fetchall()
        if results:
            for result in results:
                if name.lower() == result[1].lower():
                    return result[0]
        dev_insert_sql = '''
            insert into dev_pub_er (developer_name,about,img) values (%s,%s,%s);
        '''
        response = requests.get(url=url, **web_config)
        if response.status_code == 200:
            try:
                soup = BeautifulSoup(response.text, 'lxml')
                img = soup.select_one('#curator_avatar_image > a > img')
                if img:
                    img = img.get('src').strip()
                index = url.find('?')
                url_about = url[:index] + '/about'
                response = requests.get(url=url_about, **web_config)
                if response.status_code == 200:
                    response.encoding = 'utf-8'
                    soup = BeautifulSoup(response.text, 'lxml')
                    about = soup.select_one(
                        '#responsive_page_template_content > '
                        'div.page_content_ctn.light_container.bezel.creator_announcement_browse_adjustment > div > '
                        'div.about_container > div.desc > p')
                    if about:
                        about = about.text.strip()
                    else:
                        about = None
                else:
                    about = None
                developer_id = Wrapper(-1)
                insert_sql(db, dev_insert_sql, (name, about, img), developer_id)
                ans = developer_id.value
                return ans
            except Exception as e:
                print(f"Error: {e}")
                return -1
        else:
            time.sleep(60)
    except Exception as e:
        if isinstance(e, TimeoutException):
            print("页面加载超时")
            time.sleep(60)
        print("find_div Error")
        print(f"Error :{e}")
        return -1


# 爬取游戏主要信息 完善剩余所有表
def check_none(ob) -> bool:
    return ob is None


def get_detail(db: mysql.connections.Connection):
    AppIDs = find_appid_in_game_library(db)
    for AppID in AppIDs:
        status = static_get_game_details(db, AppID)
        if status != 200:
            print(f"SSL error in {AppID}")
            time.sleep(60)
            static_get_game_details(db, AppID)


def fix(db: mysql.connections.Connection):
    AppID_all = find_appid_in_game_library(db)
    AppID_have = find_appid_in_game(db)
    AppID_not_have = list(set(AppID_all) - set(AppID_have))
    print(AppID_not_have)
    for AppID in AppID_not_have:
        status = static_get_game_details(db, AppID)
        if status != 200:
            print(f"SSL error in {AppID}")
            time.sleep(60)
            static_get_game_details(db, AppID)


def fool_fix(db: mysql.connections.Connection):
    AppID_all = find_appid_in_game_library(db)
    AppID_have = find_appid_in_game(db)
    AppID_not_have = list(set(AppID_all) - set(AppID_have))
    print(AppID_not_have)
    for AppID in AppID_not_have:
        try:
            with open(f"data/{AppID}.html", "r", encoding='utf-8') as file:
                html = file.read()
            static_get_game_details(db, AppID, html)
        except Exception as e:
            print(f"Error: {e}")


def static_get_game_details(db: mysql.connections.Connection, AppID: int, html=None) -> int:
    statu_code = 200
    try:
        url = url_config['game_detail_url'].format(AppID)
        response = requests.get(url=url, **web_config)
        statu_code = response.status_code
        if html is not None:
            statu_code = 200
        if statu_code == 200:
            if html is None:
                response.encoding = 'utf-8'
                soup = BeautifulSoup(response.text, 'lxml')
            else:
                soup = BeautifulSoup(html, 'lxml')
            sql_insert_game = '''
                    insert into game values (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)
                '''
            game_name = soup.find('div', id='appHubAppName')
            Old_price = soup.find('div', class_='discount_original_price')
            New_price = soup.find('div', class_='discount_final_price')
            game_intro_img = soup.find('img', class_='game_header_image_full')
            divs_with_ul = soup.find('div', class_='sys_req')
            if check_none(game_name) or check_none(Old_price) or check_none(New_price) or check_none(
                    game_intro_img) or check_none(divs_with_ul):
                page_source = get_game_details(AppID)
                if page_source is None:
                    return 0
                soup = BeautifulSoup(page_source, 'lxml')
            if soup is None:
                return 0
            game_name = soup.find('div', id='appHubAppName').text.strip()
            divs_with_ul = soup.find('div', class_='sys_req')
            divs = divs_with_ul.find_all('ul')
            os = ''
            for div in divs:
                os = os + div.prettify()
            game_intro_img = soup.find('img', class_='game_header_image_full').get('src').strip()
            Recent_reviews = soup.select_one('#userReviews > div:nth-child(1)').get('data-tooltip-html').strip()
            All_reviews = soup.select_one('#userReviews > div:nth-child(2)')
            if All_reviews:
                All_reviews = All_reviews.get('data-tooltip-html').strip()
            else:
                All_reviews = Recent_reviews
            Old_price = soup.find('div', class_='discount_original_price')
            if Old_price:
                Old_price = extract_numbers_with_comma_and_dot(Old_price.text.strip())
            New_price = soup.find('div', class_='discount_final_price')
            if New_price:
                New_price = extract_numbers_with_comma_and_dot(New_price.text.strip())
            if Old_price is None and New_price is None:
                Old_price = New_price = extract_numbers_with_comma_and_dot(
                    soup.find('div', class_='game_purchase_price').text.strip())
            Date = soup.select_one(
                '#game_highlights > div.rightcol > div > div.glance_ctn_responsive_left > div.release_date > '
                'div.date')
            if Date is None:
                Date = '1970-01-01'
            else:
                Date = Date.text.strip()
            print(AppID, Date)
            issue_date = parse_date(Date)
            language = soup.find('table', class_='game_language_options').prettify()

            developer = soup.select_one('#developers_list > a').text.strip()
            publisher = soup.select_one(
                '#game_highlights > div.rightcol > div > div.glance_ctn_responsive_left > div:nth-child(4) > '
                'div.summary.column > a')
            if publisher is not None:
                publisher = publisher.text.strip()
            else:
                publisher = developer
            developer_url = soup.select_one('#developers_list').find('a').get('href')
            if publisher != developer:
                publisher_url = soup.select_one(
                    '#game_highlights > div.rightcol > div > div.glance_ctn_responsive_left > div:nth-child(4) > '
                    'div.summary.column').find('a').get('href')
            else:
                publisher_url = developer_url
            publisher_id = find_dev(db, publisher, publisher_url)
            developer_id = find_dev(db, developer, developer_url)

            insert_sql(db, sql_insert_game, (
                AppID, game_name, os, game_intro_img, Recent_reviews, All_reviews, Old_price, New_price, issue_date,
                developer_id, publisher_id, language))
            sql_develop_to_game = '''
                    insert into developer_to_game values (%s,%s)
                '''
            insert_sql(db, sql_develop_to_game, (publisher_id, AppID))
            if publisher_id != developer_id:
                insert_sql(db, sql_develop_to_game, (developer_id, AppID))
            tag_list = []
            tags = soup.find_all('a', class_='app_tag')
            for tag in tags:
                tag_list.append(tag.text.strip())
            sql_find_tag = '''
                    select tag_id,tag_name from tag;
                '''
            cursor = db.cursor()
            cursor.execute(sql_find_tag)
            tags = cursor.fetchall()
            # print(tags)
            tags_dict = {t[1]: t[0] for t in tags}
            for tag in tag_list:
                if tag in tags_dict.keys():
                    tag_id = tags_dict[tag]
                else:
                    tag_insert_sql = '''
                            insert into tag (tag_name) values (%s);
                        '''
                    tag_idw = Wrapper(-1)
                    insert_sql(db, tag_insert_sql, (tag,), tag_idw)
                    tag_id = tag_idw.value
                sql_insert_game_to_tag = '''
                        insert into game_to_tag values (%s,%s);
                    '''
                insert_sql(db, sql_insert_game_to_tag, (AppID, tag_id))
            static_content(db, AppID)
            print(f"成功插入{game_name}")
            return statu_code
        else:
            return statu_code
    except Exception as e:
        print({f"插入{AppID}失败"})
        logging.error("An error occurred: %s", str(e), exc_info=(type(e), e, e.__traceback__))
        return statu_code


def static_content(db: mysql.connections.Connection, AppID: int):
    url = url_config['content_url'].format(AppID)
    response = requests.get(url, **web_config)
    if response.status_code == 200:
        json_content = response.json()
        soup = BeautifulSoup(json_content['html'], 'lxml')
        contents = soup.find_all('div', class_='content')
        Dates = soup.find_all('div', class_='postedDate')
        sql_insert_content = '''
                                        insert into content (content_content, content_date) values (%s,%s);
                                    '''
        for Data, content in zip(Dates, contents):
            Date = Data.text.strip()
            index1 = Date.find('\n')
            index2 = Date.find('\t')
            Date = Date[:min(index1, index2) + 1]
            content = content.text.strip()
            last_id = Wrapper(-1)
            insert_sql(db, sql_insert_content, (content, Date), last_id)
            sql_insert_game_to_content = '''
                insert into content_to_game values (%s,%s)
            '''
            insert_sql(db, sql_insert_game_to_content, (last_id.value, AppID))


# selenium动态爬取
def get_game_details(AppID: int):
    driver = webdriver.Chrome()
    driver.get(f'https://store.steampowered.com/app/{AppID}/')
    try:
        if driver.current_url == f'https://store.steampowered.com/agecheck/app/{AppID}/':
            WebDriverWait(driver, 60).until(
                EC.presence_of_element_located((By.CSS_SELECTOR, '#view_product_page_btn > span')))
            WebDriverWait(driver, 60).until(
                EC.presence_of_element_located((By.CSS_SELECTOR, '#ageYear')))
            select_element = driver.find_element(By.CSS_SELECTOR, '#ageYear')
            select = Select(select_element)
            select.select_by_value('2000')
            link_element = driver.find_element(By.CSS_SELECTOR, '#view_product_page_btn > span')
            link_element.click()
        time.sleep(20)
        driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")
        time.sleep(5)
        driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")
        time.sleep(20)
    except TimeoutException:
        print("TimeoutException")
        # 游戏page
    page_source = driver.page_source
    driver.quit()
    return page_source
