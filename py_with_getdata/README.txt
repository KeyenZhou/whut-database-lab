my_spider/
├── main.py                # 主程序入口，启动爬虫
├── config.py              # 存放配置信息，如爬取目标、请求头等
├── requirements.txt       # 项目依赖的 Python 包列表
├── utils/
│   ├── __init__.py
│   ├── helpers.py         # 存放一些通用的辅助函数
│   └── constants.py       # 存放一些常量
├── data/
│   ├── __init__.py
│   └── output/
│       ├── __init__.py
│       └── results.csv    # 存放爬取结果
├── logs/
│   ├── __init__.py
│   └── spider.log         # 存放日志文件
├── spiders/
│   ├── __init__.py
│   ├── base_spider.py     # 爬虫基类，定义通用的爬取逻辑
│   ├── site1_spider.py    # 具体的爬虫实现，针对不同网站的爬取逻辑
│   └── site2_spider.py
└── tests/
    ├── __init__.py
    └── test_spiders.py     # 测试爬虫的代码
