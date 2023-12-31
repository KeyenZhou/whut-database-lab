#ifndef QQUERY_TO_DATABASE_H
#define QQUERY_TO_DATABASE_H

#include<QMap>
#include<QPair>
#include<QDate>
#include<vector>
#include<QDebug>
#include<QThread>
#include<qstring.h>
#include<QSqlDatabase>
#include<QtTest/QTest>
#include<QtSql/QSqlQuery>
#include<Qtsql/qsqlrecord.h>
#include<QtConcurrent/QtConcurrent>

#include <QNetworkAccessManager>    //网页请求方法管理类
#include <QNetworkReply>            //网页应答数据的类型，提供数据获取的方法
#include <QNetworkRequest>          //网页请求管理
#include <QJsonDocument>            //json文本
#include <QJsonArray>               //json数组
#include <QJsonObject>              //json对象
#include <QJsonValue>               //json的value数值
#include <QEventLoop>



class HUser;

class Translation
{
private:
    QNetworkAccessManager manager;
    QNetworkReply *reply;
public:
    void put_lable(QString s);
    QString get_lable();
};



struct Brief_information_of_game
{
    //游戏ID
    int appid;
    //图片的网址
    QString image_URL;
    //名字
    QString name;
    //所支持的操作系统
    QString support_os;
    //发行日期(日期类型)
    QDate shelves_time;
    //发行日期(字符串类型)
    QString date;
    //好评数量
    int number_positive_reviews;
    //差评数量
    int number_negative_reviews;
    //好评率(字符串(例：96%))
    QString positive_rate;
    //好评比(小数(例:0.96))
    double favorable_comparison;
    //折扣后的价格
    double new_price;
    //原始价格
    double old_price;
    //折扣(字符串类型(例：-40%))
    QString discount;
    //折扣率
    double discount_rate;
    //变化
    QString change;
};

struct Additional_information_of_game
{
    //近期
    //好评数量
    int number_positive_reviews;
    //差评数量
    int number_negative_reviews;
    //好评率(字符串(例：96%))
    QString positive_rate;
    //好评比(小数(例:0.96))
    double favorable_comparison;
    //发行商
    QString publisher;
    //开发商
    QString developer;
    //评价状态
    QString evaluate;
    //支持语言
    std::vector<QString> language_support;
    //游戏标签
    std::vector<QString> label_list;
    //系统配置
    QString system_configuration;
};

struct Detail_information_of_game
{
    Brief_information_of_game      brief;
    Additional_information_of_game additional;
};


class Qquery_to_database
{
private:
    const QString transaction_begin      ="BEGIN;";
    const QString transaction_submission ="COMMIT";
    const QString transaction_rollback   ="ROLLBACK;";
    QString main_query_sql;                                                         //主查寻执行的sql语句
    QString label_query_sql;                                                        //标签查询执行的sql语句
    QString ranking_query_sql;                                                      //排行查询函数的sql语句
    QString new_product_query_sql;                                                  //新品查询函数的sql语句
    QString preferential_query_sql;                                                 //优惠查询函数的sql语句
    std::vector<std::vector<int>*> information_game_sequence;                       //储存排序信息的数组列表
    QSqlQuery *query;                                                                //查询结构体
    QSqlDatabase *connect_to_stream;                                                 //连接结果体


    std::vector<Brief_information_of_game*> brief_information_of_game_list[8];      //所查询的简略游戏信息结构体的列表

    void create_information_of_game(int index);

    void date_sort();
    void date_desc_sort();
    void price_sort_ascend();
    void price_sort_descend();
    void favorable_comparison_sort();
    void favorable_comparison_desc_sort();



    //获取游戏信息结构体的顺序列表
    //日期从晚到前
    std::vector<int> get_date_sequence();
    //日期从前到晚
    std::vector<int> get_date_desc_sequence();
    //价格从低到高
    std::vector<int> get_price_ascend_sequence();
    //价格从高到低
    std::vector<int> get_price_descend_sequence();
    //好评从高到低
    std::vector<int> get_favorable_comparison_sequence();
    //好评从低到高
    std::vector<int> get_favorable_comparison_desc_sequence();

    int size_of_result;                                                             //查询结果集的大小
    int size_of_label_query;
    int size_of_weekrank_query;
    int size_of_monthrank_query;
    int size_of_yearrank_query;
    int size_of_promotion_query;
    int size_of_newproduct_query;
    int size_of_preferential_query;



    bool add_label_query_time(QString label);

public:


    Qquery_to_database();
    ~Qquery_to_database();

    void initialization();                                                          //初始化

    bool connection(HUser* user);                                                              //连接


    //0成功，1失败
    int  main_query(QString label,int user_id);                                                 //主查询函数
    bool ranking_query(int index,QString date);                                     //排行榜查询
    bool label_filter_query(QString label);                                         //标签查询函数
    bool new_product_query();                                                       //新品查询函数
    bool preferential_query();                                                      //优惠查询函数



    //参数：所查询游戏的appid
    //返回值：含有游戏信息的结构体
    Detail_information_of_game get_detail_information_of_game(int appid);           //获取游戏详细信息

    bool continue_promotion_query();

    std::vector<Brief_information_of_game> get_main_query_game(int left,int right);               //获取主查寻结果left到right之间的游戏信息
    std::vector<Brief_information_of_game> get_default_label_query_game(int left,int right);      //获取标签查询默认结果~
    std::vector<Brief_information_of_game> get_date_label_query_game(int left,int right);         //获取标签查询按日期结果~
    std::vector<Brief_information_of_game> get_date_desc_label_query_game(int left,int right);    //获取标签查询按日期结果
    std::vector<Brief_information_of_game> get_price_ascend_label_query_game(int left,int right); //获取标签查询按价格升序结果~
    std::vector<Brief_information_of_game> get_price_descend_label_query_game(int left,int right);//获取标签查询按价格降序结果~
    std::vector<Brief_information_of_game> get_favorable_comparison_label_query_game(int left,int right);  //获取标签查询按好评率结果~
    std::vector<Brief_information_of_game> get_favorable_comparison_desc_label_query_game(int left,int right);//获取标签查询按好评率结果
    std::vector<Brief_information_of_game> get_week_rank_query_game(int left,int right);          //获取周排行查询结果~
    std::vector<Brief_information_of_game> get_month_rank_query_game(int left,int right);         //获取月排行查询结果~
    std::vector<Brief_information_of_game> get_year_rank_query_game(int left,int right);          //获取年排行查询结果~
    std::vector<Brief_information_of_game> get_new_product_query_game(int left,int right);        //获取新品查询结果~
    std::vector<Brief_information_of_game> get_preferential_query_game(int left,int right);       //获取优惠查询结果~
    std::vector<QPair<QString,QString>>    get_system_label_list(int count);
};


class HUser
{
private:
    int     user_id;
    QString user_account;
    QString user_password;
    QString user_gender;
    QString user_email;
    int     user_status;
    int     user_admin;

    QSqlQuery query;                                                                //查询结构体
    QSqlDatabase connect_to_steam;                                                 //连接结果体

    const QString transaction_begin      ="BEGIN;";
    const QString transaction_submission ="COMMIT";
    const QString transaction_rollback   ="ROLLBACK;";
public:
    HUser();
    ~HUser();
    QSqlQuery* get_query();
    QSqlDatabase* get_connect_to_steam();


    int test();

    //参数：账号user_account，密码user_password
    //返回值：成功0，账号不存在1，密码不正确2
    int user_log_in(QString user_account,QString user_password);

    //参数：账号user_account，密码user_password，邮箱user_email默认为none
    //返回值：账号已存在1，成功0,注册失败原因未知（数据库内部可能出现问题）2,账号格式不对3，密码格式不对4
    int user_register(QString user_account,QString user_password,QString user_gender,QString user_email="none");

    //参数：用户id user_id
    //返回值：成功0，账号不存在1
    int user_log_out(int user_id);

    //参数：用户id user_id
    //返回值：成功0，账号不存在1
    int user_change_information(int user_id,QString user_account,QString user_gender,QString user_email);

    //返回值：例如：<user_id>2</user_id><user_account>cahs</user_account><user_password>sioc</user_password><user_gender>男</user_gender><user_email>none</user_email>
    QString get_user_information();
    int     get_user_id();
    QString get_user_account();
    QString get_user_password();
    QString get_user_gender();
    QString get_user_email();

    //获取用户定义的标签
    //返回值：例如：std::vector("car", "tea")
    std::vector<QString> get_user_tag();
    std::vector<QString> get_query_tag();

    //获取用户查询记录
    //返回值：例如：std::vector(std::pair("rlr","2023-12-25"))
    std::vector<QPair<QString,QString>> get_user_query_record();

    //获取用户评论
    //返回值：例如：std::vector(std::vector("wonderful", "2023-12-25", "Euro Truck Simulator 2", "227300"), std::vector("112", "2023-12-25", "The Elder Scrolls® Online", "306130"))
    //std::vector("wonderful", "2023-12-25", "Euro Truck Simulator 2", "227300")
    //第一个是评论内容，第二个是评论日期，第三个是评论对应的游戏名，第四个是该游戏的appid
    std::vector<std::vector<QString>> get_user_content();

    //存储用户查询记录
    //参数：查询的信息
    //返回值：0成功 ，小于0失败
    int insert_user_qurty_information(QString label);

    //0失败，1成功
    int add_user_label(QString label,int appid);
    //
    int delete_user_label(QString label_name);

    int add_user_content(QString content,int appid);

    int delete_user_content(QString content);
};

#endif // QQUERY_TO_DATABASE_H
