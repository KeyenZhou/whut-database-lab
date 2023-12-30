import argparse
import datetime

import pymysql as mysql
from bs4 import BeautifulSoup
import requests
import time
from dateutil import parser

db_config = {
    'host': 'localhost',
    'user': 'root',
    'password': '523902',
    'database': 'steam'
}

url_config = {
    'game_library_url': "https://store.steampowered.com/search/?specials=1&page={}",
    'game_library_num': 40,

    'week_rank_url': "https://store.steampowered.com/charts/topsellers/CN/{}",

    'month_rank_url': "https://store.steampowered.com/charts/topnewreleases/top_{}_2023",

    'year_rank_url': 'https://store.steampowered.com/charts/bestofyear/BestOf2022',

    'game_detail_url': "https://store.steampowered.com/app/{}/",

    'content_url': "https://store.steampowered.com/appreviews/{}?cursor=*",

    'start_date': '2023-11-07',
    'end_date': '2023-12-12',
}

web_config = {
    'headers': {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) '
                      'Chrome/120.0.0.0 Safari/537.36'
    },
    'proxies': {
        'http': 'http://127.0.0.1:7890',
        'https': 'http://127.0.0.1:7890'
    },
    'params': {
        'wd': 'ip'
    },
}


class Wrapper:
    def __init__(self, value):
        self.value = value


# 创建 ArgumentParser 对象
Aparser = argparse.ArgumentParser(description='此脚本用于查找特定的appid到数据库中')

# 添加参数
Aparser.add_argument('arg1', type=int, help='AppID')

# 解析命令行参数
args = Aparser.parse_args()

# 获取参数的值
arg1_value = args.arg1

sql_find_appid_is_exist = f'select * from game_library where AppID = {arg1_value}'

db = mysql.connect(**db_config)

cursor = db.cursor()


def insert_sql(db: mysql.connections.Connection, sql: str, params: tuple, lastrowid=Wrapper(-1)) -> None:
    """
    插入数据库的函数

    Args:
        db (pymysql.connections.Connection): pymysql的连接
        sql(str): sql语句
        params(tuple): sql语句中占位符的参数
        lastrowid(Wrapper): 返回最后一次插入时的自增主键的值

    Returns:
        None: 这个函数没有返回值

    Raises:
        ValueError: 如果参数不符合预期，可能会引发 ValueError 异常。

    Example:
        >>> insert_sql(db,sql,params,lastrowid)
        None
    """
    cursor = db.cursor()
    try:
        cursor.execute(sql, params)
        lastrowid.value = cursor.lastrowid
        db.commit()
    except Exception as e:
        print(f'Error: {e}')
        print(sql)
        db.rollback()
    finally:
        cursor.close()


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


def parse_date(date_string):
    try:
        # 尝试使用 dateutil.parser.parse 解析
        parsed_date = parser.parse(date_string)
        return parsed_date.strftime("%Y-%m-%d")
    except ValueError:
        try:
            # 尝试使用 datetime.strptime 解析中文日期
            chinese_date_format = "%Y 年 %m 月 %d 日"
            parsed_date = datetime.strptime(date_string, chinese_date_format)
            return parsed_date.strftime("%Y-%m-%d")
        except ValueError:
            return None


def find_dev(db: mysql.connections.Connection, name: str, url: str) -> int:
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
            print("插入开发商成功")
            ans = developer_id.value
            return ans
        except Exception as e:
            print(f"Error: {e}")
            return -1


def get_game_detail(db: mysql.connections.Connection, AppID: int):
    url = url_config['game_detail_url'].format(AppID)
    response = requests.get(url=url, **web_config)
    if response.status_code == 200:
        response.encoding = 'utf-8'
        soup = BeautifulSoup(response.text, 'lxml')
        sql_insert_game = '''
                        insert into game values (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)
                    '''
        game_name = soup.find('div', id='appHubAppName').text.strip()
        divs_with_ul = soup.find('div', class_='sys_req')
        divs = divs_with_ul.find_all('ul')
        os_list = []
        for div in divs:
            os_list.append(div.prettify())
        os = '#'.join(os_list)
        game_intro_img = soup.find('img', class_='game_header_image_full').get('src').strip()
        print("img")
        Recent_reviews = soup.select_one('#userReviews > div:nth-child(1)').get('data-tooltip-html').strip()
        print("recent")
        All_reviews = soup.select_one('#userReviews > div:nth-child(2)')
        if All_reviews:
            All_reviews = All_reviews.get('data-tooltip-html').strip()
        else:
            All_reviews = Recent_reviews
        print("all")
        Old_price = soup.find('div', class_='discount_original_price').text.strip()
        New_price = soup.find('div', class_='discount_final_price').text.strip()
        Date = soup.select_one(
            '#game_highlights > div.rightcol > div > div.glance_ctn_responsive_left > div.release_date > '
            'div.date').text.strip()
        print(AppID, Date)
        issue_date = parse_date(Date)

        developer = soup.select_one('#developers_list > a').text.strip()
        publisher = soup.select_one(
            '#game_highlights > div.rightcol > div > div.glance_ctn_responsive_left > div:nth-child(4) > '
            'div.summary.column > a').text.strip()
        developer_url = soup.select_one('#developers_list').find('a').get('href')
        publisher_url = soup.select_one(
            '#game_highlights > div.rightcol > div > div.glance_ctn_responsive_left > div:nth-child(4) > '
            'div.summary.column').find('a').get('href')
        publisher_id = find_dev(db, publisher, publisher_url)
        developer_id = find_dev(db, developer, developer_url)

        insert_sql(db, sql_insert_game, (
            AppID, game_name, os, game_intro_img, Recent_reviews, All_reviews, Old_price, New_price, issue_date,
            developer_id, publisher_id))
        print(f"插入游戏:{game_name} 成功")
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
        print(f"插入对应标签成功")
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
    else:
        time.sleep(60)


try:
    cursor.execute(sql_find_appid_is_exist)
    result = cursor.fetchone()
    if result is not None:
        print("游戏库中存在此游戏")
    else:
        AppID = arg1_value
        get_game_detail(db, AppID)
except Exception as e:
    print(f"Error {e}")
