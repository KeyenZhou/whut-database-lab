db_config = {
    'host': 'localhost',
    'user': 'root',
    'password': '523902',
    'database': 'steam'
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
