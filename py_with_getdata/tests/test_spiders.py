# 测试点击:
from selenium.common import TimeoutException
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.select import Select
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import time
import requests
from bs4 import BeautifulSoup
from config import web_config
from config import url_config
from spiders.base_spider import static_content
import pymysql as mysql
from config import db_config
from spiders.base_spider import static_content

db = mysql.connect(**db_config)
static_content(db, 971650)


def test():
    driver = webdriver.Chrome()
    driver.get('https://store.steampowered.com/app/489830/The_Elder_Scrolls_V_Skyrim_Special_Edition/')
    if driver.current_url == 'https://store.steampowered.com/agecheck/app/489830/':
        WebDriverWait(driver, 60).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, '#view_product_page_btn > span')))
        WebDriverWait(driver, 60).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, '#ageYear')))
        select_element = driver.find_element(By.CSS_SELECTOR, '#ageYear')
        select = Select(select_element)
        select.select_by_value('2000')
        link_element = driver.find_element(By.CSS_SELECTOR, '#view_product_page_btn > span')
        link_element.click()
    WebDriverWait(driver, 60).until(
        EC.url_to_be('https://store.steampowered.com/app/489830/The_Elder_Scrolls_V_Skyrim_Special_Edition/'))
    driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")
    WebDriverWait(driver, 60).until(
        EC.presence_of_element_located((By.CSS_SELECTOR, '#ViewAllReviewssummary > a')))
    driver.execute_script("window.scrollTo({top: document.body.scrollHeight, behavior: 'smooth'});")
    time.sleep(20)
    captured_requests = driver.execute_script("return window.performance.getEntriesByType('resource')")
    for request in captured_requests:
        url = request['name']
        method = request['initiatorType']
        if method == 'xmlhttprequest':
            if url.startswith(f'https://store.steampowered.com/appreviews/489830?cursor='):
                response = requests.get(url)
                json_data = response.json()
                soup = BeautifulSoup(json_data['html'], 'lxml')
                contents = soup.find_all('div', class_='content')
                for content in contents:
                    print(content.text.strip())
    driver.quit()
