# 辅助类,用于传入引用参数
from datetime import datetime

import pymysql as mysql
from dateutil import parser

import re


class Wrapper:
    def __init__(self, value):
        self.value = value


# 插入数据库
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


def select_sql(db: mysql.connections.Connection, sql: str, params: tuple):
    """
    查询数据库的函数

    Args:
        db (pymysql.connections.Connection): pymysql的连接
        sql(str): sql语句
        params(tuple): sql语句中占位符的参数

    Returns:
        tuple[tuple[Any,...]...]: 返回元组的元组
        None: 没有查到

    Raises:
        ValueError: 如果参数不符合预期，可能会引发 ValueError 异常。

    Example:
        >>> select_sql(db,sql,params)
        None
    """
    cursor = db.cursor()
    ans = None
    try:
        cursor.execute(sql, params)
        ans = cursor.fetchall()
        db.commit()
    except Exception as e:
        print(f'Error: {e}')
        print(sql)
        db.rollback()
    finally:
        cursor.close()
        return ans


# 创建数据库表
def create_schema(db: mysql.connections.Connection) -> None:
    """
    创建数据库的函数

    Args:
        db (pymysql.connections.Connection): pymysql的连接

    Returns:
        None: 这个函数没有返回值

    Raises:
        ValueError: 如果参数不符合预期，可能会引发 ValueError 异常。

    Example:
        >>> create_schema(db)
        None
    """
    with open(r"../Steam_create.sql", mode="r", encoding="utf-8") as file:
        create_database_sql = file.read()
    cursor = db.cursor()
    try:
        sql_commands = create_database_sql.split(';')
        for command in sql_commands:
            command = command.strip()
            if command:
                cursor.execute(command + ';')
                db.commit()
    except Exception as e:
        print(f"Error message: {e}")
        db.rollback()
    finally:
        cursor.close()


# 查询appid是否在game_library表中
def query_appid(db: mysql.connections.Connection, AppID: int) -> bool:
    cursor = db.cursor()
    query = f"SELECT * FROM game_library WHERE AppID = '{AppID}'"
    cursor.execute(query)
    result = cursor.fetchone()
    if result:
        return True
    else:
        return False


# 找到游戏库中所有的appid
def find_appid_in_game_library(db: mysql.connections.Connection) -> list:
    sql = '''
    select AppID from game_library
    '''
    results = []
    cursor = db.cursor()
    try:
        cursor.execute(sql)
        results = cursor.fetchall()
    except Exception as e:
        print(f"Error while fetching game_library, error: {e}")
    finally:
        cursor.close()
    results = [result[0] for result in results]
    return results


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


# 转换money
def extract_numbers_with_comma_and_dot(input_string):
    # 使用正则表达式匹配包含逗号、点号和数字的子串
    matches_money = re.findall(r'[,.\d]+', input_string)
    matches_symbol = re.findall(r'[^,.\d]+', input_string)

    # 将匹配到的字符串列表连接成一个字符串
    result_money = ''.join(matches_money)
    result_symbol = ''.join(matches_symbol)

    return f"{result_symbol}({result_money})"
