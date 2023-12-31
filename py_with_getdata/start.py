import pymysql as mysql
from spiders.base_spider import get_game_library
from config import db_config
from utils.helpers import create_schema
from spiders.base_spider import get_week_rank
from spiders.base_spider import get_month_rank
from spiders.base_spider import get_year_rank
from spiders.base_spider import get_detail
from utils.helpers import insert_sql
from spiders.base_spider import fix
from spiders.base_spider import fool_fix

if __name__ == '__main__':
    db = mysql.connect(**db_config)
    # # 创建数据库
    # # create_schema(db)
    # user_sql = '''
    #     insert into user (user_id, user_account, user_password, user_gender, user_email, user_status, user_admin) values (%s,%s,%s,%s,%s,%s,%s)
    # '''
    # insert_sql(db, user_sql, (1, '1', '1', '1', '1', '1', '1'))
    # # 获得基本游戏数据
    # get_game_library(db)
    #
    # # 获得排行
    # get_week_rank(db)
    # get_month_rank(db)
    # get_year_rank(db)

    # 获得其他详细信息
    # get_detail(db)

    # 修补
    # fix(db)

    # 傻瓜修补
    fool_fix(db)
    db.close()
