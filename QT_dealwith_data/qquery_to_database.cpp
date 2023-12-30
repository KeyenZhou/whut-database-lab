#include "qquery_to_database.h"

void analysis_reviews(Brief_information_of_game* one,QString Good);
QString analysis_os(QString os);
/*----------------------------------------------Translation----------------------------------------------*/
void Translation::put_lable(QString s)
{
    QString API = "http://api.fanyi.baidu.com/api/trans/vip/translate?q="
                  +s.toUtf8()+"&from=zh&to=en&appid=20231219001915339&salt=1435660288&sign=";
    QString MD5;
    QString passWd="20231219001915339"+s.toUtf8()+"1435660288dt7yhhzPQQwNLO5WLgBo";
    QByteArray str;
    str = QCryptographicHash::hash(passWd.toUtf8(),QCryptographicHash::Md5);
    MD5.append(str.toHex());
    API=API+MD5;

    QNetworkRequest quest;
    quest.setUrl(QUrl(API));
    reply=manager.get(quest);
}

QString Translation::get_lable()
{
    QString all = reply->readAll();
    QString mean = "";
    QJsonParseError jsonError;
    QJsonDocument json = QJsonDocument::fromJson(all.toUtf8(),&jsonError);
    if (jsonError.error == QJsonParseError::NoError)
    {
        if (json.isObject())
        {
            QJsonObject rootObj = json.object();
            if(rootObj.contains("trans_result"))
            {
                QJsonValue resultValue = rootObj.value(QString("trans_result"));
                QJsonArray array = resultValue.toArray();
                for(int i = 0;i < array.size();i++)
                {
                    QJsonObject explains = array.at(i).toObject();
                    if(explains.contains("dst"))
                    {
                        QJsonValue dst = explains.value(QString("dst"));
                        mean = mean + dst.toString();
                    }
                }
            }
        }
    }

    return mean;

}

/*----------------------------------------------User----------------------------------------------*/

QSqlQuery* User::get_query()
{
    return &query;
}
QSqlDatabase* User::get_connect_to_steam()
{
    return &connect_to_steam;
}

User::User()
{
    connect_to_steam = QSqlDatabase::addDatabase("QODBC");
    //connect_to_steam.setHostName("114.116.248.95");
    connect_to_steam.setHostName("127.0.0.1");
    connect_to_steam.setPort(3306);
    connect_to_steam.setDatabaseName("steam");
    connect_to_steam.setUserName("root");
    //connect_to_steam.setPassword("123456");
    connect_to_steam.setPassword("wangisgreat.0525");
    query=QSqlQuery(connect_to_steam);
    connect_to_steam.open();

    query.exec(transaction_begin);
    query.exec("create procedure one(out sum int)\n"
               "begin set sum = LAST_INSERT_ID(); end;");
    query.exec(transaction_submission);
}

User::~User()
{
    connect_to_steam.close();
}

int User::user_log_in(QString user_account1,QString user_password1)
{
    query.exec(transaction_begin);

    QString user_log_in_sql = "select user_id,\n"
                              "user_account,\n"
                              "user_password,\n"
                              "user_gender,\n"
                              "user_email,\n"
                              "user_status,\n"
                              "user_admin from user where user_account=:user_account and user_status!=-1";
    query.prepare(user_log_in_sql);
    query.bindValue(":user_account",user_account1);
    query.exec();
    if(query.numRowsAffected()==0)
    {
        query.exec(transaction_submission);
        return 1;
    }
    query.next();
    if(query.value("user_password").toString()==user_password1&&query.value("user_status").toInt()==0)
    {
        user_id       =query.value("user_id").toInt();
        user_account  =query.value("user_account").toString();
        user_password =query.value("user_password").toString();
        user_gender   =query.value("user_gender").toString();
        user_email    =query.value("user_email").toString();
        user_status   = 1;
        user_admin    =query.value("user_admin").toInt();
        query.exec(transaction_submission);
        return 0;
    }
    query.exec(transaction_submission);
    return 2;
}

int User::user_register(QString user_account,QString user_password,QString user_gender,QString user_email)
{
    query.exec(transaction_begin);

    QString a="select count(*) from user where user_account=:user_account and user_status!=-1;";
    query.prepare(a);
    query.bindValue(":user_account",user_account);
    query.exec();
    query.next();
    if(query.value(0).toInt()>0)
    {
        query.exec(transaction_submission);
        return 1;
    }

    QString user_register_sql = "insert into user(user_account,user_password,user_gender,user_email) \n"
                                "value(:user_account,:user_password,:user_gender,:user_email);";
    query.prepare(user_register_sql);
    query.bindValue(":user_account",user_account);
    query.bindValue(":user_password",user_password);
    query.bindValue(":user_gender",user_gender);
    query.bindValue(":user_email",user_email);
    query.exec();
    qDebug()<<query.numRowsAffected();
    if(query.numRowsAffected()==1)
    {
        query.exec(transaction_submission);
        return 0;
    }
    else
    {
        query.exec(transaction_rollback);
        return 2;
    }
}

int User::user_log_out(int user_id)
{
    query.exec(transaction_begin);
    QString user_log_out_sql = "update user set user_status=-1 where user_id=:user_id";
    query.prepare(user_log_out_sql);
    query.bindValue(":user_id",user_id);
    query.exec();
    if(query.numRowsAffected()==0)
    {
        query.exec(transaction_rollback);
        return 1;
    }
    else
    {
        query.exec(transaction_submission);
        return 0;
    }
}

int User::user_change_information(int user_id,QString user_password,QString user_gender,QString user_email)
{
    QString user_change_information_sql="update user set user_password=:user_password,\n"
                                          "user_gender=:user_gender,\n"
                                          "user_email=:user_email where user_id=:user_id";
    query.prepare(user_change_information_sql);
    query.bindValue(":user_id",user_id);
    query.bindValue(":user_password",user_password);
    query.bindValue(":user_gender",user_gender);
    query.bindValue(":user_email",user_email);
    query.exec();
    if(query.numRowsAffected()==0) return 1;
    else return 0;
}


QString User::get_user_information()
{

    return  "<user_id>"+QString::number(user_id)+"</user_id>"+
            "<user_account>"+user_account+"</user_account>"+
            "<user_password>"+user_password+"</user_password>"+
            "<user_gender>"+user_gender+"</user_gender>"+
            "<user_email>"+user_email+"</user_email>";
}

int User::get_user_id()
{
    return user_id;
}

QString User::get_user_account()
{
    return user_account;
}

QString User::get_user_password()
{
    return user_password;
}

QString User::get_user_gender()
{
    return user_gender;
}

QString User::get_user_email()
{
    return user_email;
}

std::vector<QString> User::get_user_tag()
{
    query.exec(transaction_begin);
    QString get_user_tag_sql =  "select tag_name from tag,\n"
                                "(select tag_id from user_to_tag where user_id=:user_id) as a\n"
                                "where tag.tag_id=a.tag_id order by tag_times desc";
    query.prepare(get_user_tag_sql);
    query.bindValue(":user_id",user_id);
    query.exec();
    std::vector<QString> tag_list;
    while(query.next())
    {
        tag_list.push_back(query.value(0).toString());
    }
    query.exec(transaction_submission);
    return tag_list;
}

std::vector<QString> User::get_query_tag()
{
    std::vector<QString> tag_list=get_user_tag();
    int need=10-tag_list.size();
    if(need>0)
    {
        QString get_system_tag_sql =  "select tag_name from tag where user_id=1 limit :need";
        query.prepare(get_system_tag_sql);
        query.bindValue(":need",need);
        query.exec();
        while(query.next())
        {
            tag_list.push_back(query.value(0).toString());
        }
    }
    return tag_list;
}

std::vector<QPair<QString,QString>> User::get_user_query_record()
{
    query.exec(transaction_begin);
    QString get_user_query_record_sql = "select query_content,query_time from query_record,\n"
                                        "(select record_ID from user_to_query where user_id=:user_id) as a\n"
                                        "where query_record.record_ID=a.record_ID;";
    query.prepare(get_user_query_record_sql);
    query.bindValue(":user_id",user_id);
    query.exec();
    std::vector<QPair<QString,QString>> query_record_list;
    while(query.next())
    {
        query_record_list.push_back(qMakePair(query.value("query_content").toString(),query.value("query_time").toDate().toString("yyyy-MM-dd")));
    }
    query.exec(transaction_submission);
    return query_record_list;
}

std::vector<std::vector<QString>> User::get_user_content()
{
    query.exec(transaction_begin);
    QString get_user_content_sql = "select content_content,content_date,c.game_name,c.AppID from content,\n"
                                   "(select content_id from content_to_user where user_id=:user_id) as a\n"
                                   "join\n"
                                   "(select game_name,b.AppID,content_id from game,\n"
                                   "(select AppID,a.content_id from content_to_game,\n"
                                   "(select content_id from content_to_user where user_id=:user_id) as a\n"
                                   "where a.content_id=content_to_game.content_id) as b\n"
                                   "where b.AppID=game.AppID) as c\n"
                                   "where content.content_id=a.content_id and content.content_id=c.content_id;";
    query.prepare(get_user_content_sql);
    query.bindValue(":user_id",user_id);
    query.exec();
    std::vector<std::vector<QString>> content_list;
    while(query.next())
    {
        std::vector<QString> content;
        content.push_back(query.value("content_content").toString());
        content.push_back(query.value("content_date").toDate().toString("yyyy-MM-dd"));
        content.push_back(query.value("game_name").toString());
        content.push_back(query.value("AppID").toString());
        content_list.push_back(content);
    }
    query.exec(transaction_submission);
    return content_list;
}

int User::insert_user_qurty_information(QString label)
{
    query.exec(transaction_begin);
    QString insert_user_qurty_information="insert into query_record(query_time, query_content) value(:curdate,:label);";
    query.prepare(insert_user_qurty_information);
    query.bindValue(":curdate",QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    query.bindValue(":label",label);
    query.exec();
    if(query.numRowsAffected()!=1)
    {
        query.exec(transaction_rollback);
        return -1;
    }
    QString get_query_id="select record_ID from query_record order by record_ID limit 1;";
    int query_id=0;
    if(query.exec(get_query_id))
    {
        query.next();
        query_id=query.value(0).toInt()+1;
    }
    else
    {
        query.exec(transaction_rollback);
        return -2;
    }
    QString insert_user_to_query_sql="insert into user_to_query(record_ID, user_id) value(:record_ID,:user_id);";
    query.prepare(insert_user_to_query_sql);
    query.bindValue(":record_ID",query_id);
    query.bindValue(":user_id",user_id);
    if(!query.exec())
    {
        query.exec(transaction_rollback);
        return -3;
    }
    query.exec(transaction_submission);
    return 0;
}

int User::add_user_label(QString label,int appid)
{
    query.exec(transaction_begin);

    QString add_user_label_sql="insert into tag(tag_name, user_id, tag_times) value (:label,:user_id,1);";
    query.prepare(add_user_label_sql);
    query.bindValue(":label",label);
    query.bindValue(":user_id",user_id);
    if(!query.exec())
    {
        query.exec(transaction_rollback);
        return 0;
    }

    int tag_id;
    query.exec("CALL one(@s);");
    query.exec("select @s");
    if(query.next()){
        tag_id=query.value(0).toInt();
    }
    QString add_user_to_tag_sql="insert into user_to_tag(user_id, tag_id) VALUE(:user_id,:tag_id);";
    query.prepare(add_user_to_tag_sql);
    query.bindValue(":user_id",user_id);
    query.bindValue(":tag_id",tag_id);
    if(!query.exec())
    {
        query.exec(transaction_rollback);
        return 0;
    }
    QString add_game_to_tag_sql="insert into game_to_tag(appid, tag_id) VALUE(:appid,:tag_id);";
    query.prepare(add_game_to_tag_sql);
    query.bindValue(":appid",appid);
    query.bindValue(":tag_id",tag_id);
    if(!query.exec())
    {
        query.exec(transaction_rollback);
        return 0;
    }

    query.exec(transaction_submission);
    return 1;
}

int User::delete_user_label(QString label_name)
{
    query.exec(transaction_begin);

    int tag_id=0;

    QString get_tag_id="select tag_id from tag where tag_name=:label";
    query.prepare(get_tag_id);
    query.bindValue(":label",label_name);
    if(!query.exec())
    {
        query.exec(transaction_rollback);
        return 0;
    }
    query.next();
    tag_id=query.value(0).toInt();

    qDebug()<<"1";
    QString delete_user_to_tag_sql="delete from user_to_tag where tag_id=:tag_id";
    query.prepare(delete_user_to_tag_sql);
    query.bindValue(":tag_id",tag_id);
    if(!query.exec())
    {
        query.exec(transaction_rollback);
        return 0;
    }
    qDebug()<<"1";
    QString delete_game_to_tag_sql="delete from game_to_tag where tag_id=:tag_id";
    query.prepare(delete_game_to_tag_sql);
    query.bindValue(":tag_id",tag_id);
    if(!query.exec())
    {
        query.exec(transaction_rollback);
        return 0;
    }
    qDebug()<<"1";
    QString delete_user_label_sql="delete from tag where tag_id=:tag_id";
    query.prepare(delete_user_label_sql);
    query.bindValue(":tag_id",tag_id);
    if(!query.exec())
    {
        query.exec(transaction_rollback);
        return 0;
    }
    qDebug()<<"1";
    query.exec(transaction_submission);

    return 1;

}


//--------------------------------------------------------------------------------------------------------------



//初始化
Qquery_to_database::Qquery_to_database()
{

}

//析构
Qquery_to_database::~Qquery_to_database()
{
    connect_to_stream->close();
    for(auto a:information_game_sequence)
    {
        std::free(a);
        a->shrink_to_fit();
    }
    for(auto a:brief_information_of_game_list)
    {
        for(auto b:a) std::free(b);
        a.shrink_to_fit();
    }
}

void Qquery_to_database::initialization()
{
    for(int i=0;i<=7;i++) brief_information_of_game_list[i].clear();
    main_query_sql.clear();
    label_query_sql.clear();
    new_product_query_sql.clear();
    preferential_query_sql.clear();


    std::vector<int>* sequence_brief_information_of_game ;

    //发行日期顺序information_game_sequence[0]
    sequence_brief_information_of_game = new std::vector<int>;
    information_game_sequence.push_back(std::move(sequence_brief_information_of_game));

    //价格低到高顺序information_game_sequence[1]
    sequence_brief_information_of_game = new std::vector<int>;
    information_game_sequence.push_back(std::move(sequence_brief_information_of_game));

    //价格低到高顺序information_game_sequence[2]
    sequence_brief_information_of_game = new std::vector<int>;
    information_game_sequence.push_back(std::move(sequence_brief_information_of_game));

    //好评率高到低顺序information_game_sequence[3]
    sequence_brief_information_of_game = new std::vector<int>;
    information_game_sequence.push_back(std::move(sequence_brief_information_of_game));

    //发行日期顺序(前晚)information_game_sequence[4]
    sequence_brief_information_of_game = new std::vector<int>;
    information_game_sequence.push_back(std::move(sequence_brief_information_of_game));

    //好评率低到高顺序information_game_sequence[5]
    sequence_brief_information_of_game = new std::vector<int>;
    information_game_sequence.push_back(std::move(sequence_brief_information_of_game));
}

//连接数据库
bool Qquery_to_database::connection(User* user)
{
    // connect_to_stream = QSqlDatabase::addDatabase("QODBC");
    // connect_to_stream->setHostName("127.0.0.1");
    // connect_to_stream->setPort(3306);
    // connect_to_stream->setDatabaseName("steam");
    // connect_to_stream->setUserName("root");
    // connect_to_stream->setPassword("wangisgreat.0525");
    // query=QSqlQuery(connect_to_stream);
    // return connect_to_stream->open();

    query=user->get_query();
    connect_to_stream=user->get_connect_to_steam();
}


// QFuture<void> future= QtConcurrent::run([this]{
//     query->exec("select * from tableone where GameName like '%car%' limit 9");
//     size_of_result=query->numRowsAffected();
//     qDebug()<<size_of_result<<"\n";
// });
// qDebug()<<size_of_result<<"\n";
// QTest::qSleep(15000);

/*----------------------------------------------------------------------------------------------------------------------------------*/
//查询函数


bool Qquery_to_database::add_label_query_time(QString label)
{
    query->exec(transaction_begin);
    QString add_label_query_time_sql="update tag,(select tag_id from tag where tag_name=:label) as a\n"
                                       "set tag.tag_times=tag.tag_times+1 where tag.tag_id =a.tag_id;";
    query->prepare(add_label_query_time_sql);
    query->bindValue(":label",label);
    if(query->exec())
    {
        query->exec(transaction_submission);
        return true;
    }
    query->exec(transaction_rollback);
    return false;
}

int Qquery_to_database::main_query(QString label,int user_id)
{
    main_query_sql.clear();
    query->clear();
    for(auto a:(brief_information_of_game_list[0])) std::free(a);
    std::vector<Brief_information_of_game*>().swap(brief_information_of_game_list[0]);

    size_of_result=0;

    query->exec(transaction_begin);


    query->exec("select tag_id from tag where tag_name = :label and user_id=:user_id;");
    query->bindValue(":label",label);
    query->bindValue(":user_id",user_id);
    query->exec();
    if(query->numRowsAffected()==0)
    {
        query->exec(transaction_submission);
        return 0;
    }

    if(label.size()!=0)
    {
        main_query_sql=  "select game.AppID,game_name,os,game_intro_img,All_reviews,Old_price,New_price,issue_date\n"
                         "from game,\n"
                         "(select distinct AppID from game_to_tag,\n"
                         "(select tag_id from tag where tag_name = :label and user_id=:user_id) as a\n"
                         "where a.tag_id=game_to_tag.tag_id) as b\n"
                         "where b.AppID=game.AppID;";
        query->prepare(main_query_sql);
        query->bindValue(":label",label);
        query->bindValue(":user_id",user_id);
        query->exec();
    }
    else
    {
        main_query_sql = "select AppID,game_name,os,game_intro_img,All_reviews,Old_price,New_price,issue_date\n"
                         "from game limit 25;";
        query->prepare(main_query_sql);
        query->exec();
    }
    create_information_of_game(0);
    query->exec(transaction_submission);

    add_label_query_time(label);
    return 1;
}

bool Qquery_to_database::label_filter_query(QString lable)
{
    int mark=-1;
    std::vector<QString> lable_list;

    for(auto i:lable)
    {
        if(i=='/')
        {
            mark++;
            lable_list.push_back("");
        }
        else lable_list[mark].append(i);
    }

    label_query_sql.clear();
    query->clear();
    for(auto a:(brief_information_of_game_list[4])) std::free(a);
    std::vector<Brief_information_of_game*>().swap(brief_information_of_game_list[4]);

    size_of_label_query=0;

    query->exec(transaction_begin);
    label_query_sql = "select game.AppID,game_name,os,game_intro_img,All_reviews,Old_price,New_price,issue_date from game,\n"
                      "(select distinct AppID from game_to_tag,\n"
                      "(select tag_id from tag where 1=1";
    std::vector<QString> parameter_label_list;
    for(int i=0;i<=mark;i++)
    {
        parameter_label_list.push_back(":biaoqian"+QString::number(i));
        label_query_sql = label_query_sql + " and tag_name = "+parameter_label_list[i];
    }
    label_query_sql=label_query_sql + ") as a\n"
                                        "where a.tag_id=game_to_tag.tag_id) as b\n"
                                        "where game.AppID=b.AppID;";
    query->prepare(label_query_sql);
    for(int i=0;i<=mark;i++)
    {
        query->bindValue(parameter_label_list[i],lable_list[i]);
    }
    query->exec();
    size_of_label_query=query->numRowsAffected();
    create_information_of_game(4);
    query->exec(transaction_submission);
    date_sort();
    date_desc_sort();
    price_sort_ascend();
    price_sort_descend();
    favorable_comparison_sort();
    favorable_comparison_desc_sort();
    return true;
}

bool Qquery_to_database::ranking_query(int index,QString date)
{
    ranking_query_sql.clear();
    query->clear();

    query->exec(transaction_begin);

    QDate time;
    if(index==0)
    {
        for(auto a:(brief_information_of_game_list[1])) std::free(a);
        std::vector<Brief_information_of_game*>().swap(brief_information_of_game_list[1]);

        time=QDate::fromString(date, "yyyy-MM-dd").addDays(1-QDate::fromString(date, "yyyy-MM-dd").dayOfWeek()).addDays(1);
        ranking_query_sql = "select game.AppID,game_name,os,game_intro_img,All_reviews,Old_price,New_price,issue_date,b.rank_change from game,\n"
                            "(SELECT\n"
                            "wr.`Date`,\n"
                            "wr.`AppID`,\n"
                            "wr.`rank`,\n"
                            "LAG(wr.`rank`) OVER (ORDER BY wr.`Date`) AS previous_rank,\n"
                            "CASE\n"
                            "WHEN wr.`rank` < LAG(wr.`rank`) OVER (ORDER BY wr.`Date`) THEN CONCAT('上升', LAG(wr.`rank`) OVER (ORDER BY wr.`Date`) - wr.`rank`)\n"
                            "WHEN wr.`rank` > LAG(wr.`rank`) OVER (ORDER BY wr.`Date`) THEN CONCAT('下降', wr.`rank` - LAG(wr.`rank`) OVER (ORDER BY wr.`Date`))\n"
                            "ELSE '新上榜'\n"
                            "END AS rank_change\n"
                            "FROM `week_rank` wr WHERE wr.`Date` = :time) as b\n"
                            "where b.AppID=game.AppID\n"
                            "ORDER BY `rank`;";
        query->prepare(ranking_query_sql);
        query->bindValue(":time",time);
        query->exec();
        create_information_of_game(1);
    }

    else if(index==1)
    {
        for(auto a:(brief_information_of_game_list[1])) std::free(a);
        std::vector<Brief_information_of_game*>().swap(brief_information_of_game_list[2]);

        time=QDate::fromString(date, "yyyy-MM-dd").addDays(1-QDate::fromString(date, "yyyy-MM-dd").day());
        ranking_query_sql = "select game.AppID,game_name,os,game_intro_img,All_reviews,Old_price,New_price,issue_date from game,\n"
                            "(select AppID from month_rank where Date=:time) as a\n"
                            "where a.AppID=game.AppID;";
        query->prepare(ranking_query_sql);
        query->bindValue(":time",time);
        query->exec();
        create_information_of_game(2);
    }

    else if(index==2)
    {
        for(auto a:(brief_information_of_game_list[1])) std::free(a);
        std::vector<Brief_information_of_game*>().swap(brief_information_of_game_list[3]);

        time=QDate::fromString(date, "yyyy-MM-dd").addDays(1-QDate::fromString(date, "yyyy-MM-dd").dayOfYear());
        ranking_query_sql = "select game.AppID,game_name,os,game_intro_img,All_reviews,Old_price,New_price,issue_date,a.category from game,\n"
                            "(select AppID,category from year_rank where Date='2022-01-01') as a\n"
                            "where a.AppID=game.AppID;";
        query->prepare(ranking_query_sql);
        query->bindValue(":time",time);
        query->exec();
        create_information_of_game(3);
    }

    query->exec(transaction_submission);

    return true;
}

bool Qquery_to_database::new_product_query()
{
    query->clear();
    for(auto a:(brief_information_of_game_list[6])) std::free(a);
    std::vector<Brief_information_of_game*>().swap(brief_information_of_game_list[6]);

    query->exec(transaction_begin);
    new_product_query_sql = "select game.AppID,game_name,os,game_intro_img,All_reviews,Old_price,New_price,issue_date\n"
                            "FROM `game`\n"
                            "WHERE `issue_date` >=\n"
                            "DATE_FORMAT(DATE_SUB((SELECT `issue_date` FROM `game` ORDER BY `issue_date` DESC LIMIT 1), INTERVAL 2 MONTH), '%Y-%m-01')\n"
                            "ORDER BY `issue_date` DESC;";
    query->prepare(new_product_query_sql);
    query->exec();
    create_information_of_game(6);
    query->exec(transaction_submission);
    return true;
}

bool Qquery_to_database::preferential_query(QString discount)
{
    QString disc;
    for(auto a:discount)
    {
        if('0'<=a and a<='9') disc.append(a);
    }
    double dis=disc.toDouble()/100;
    query->clear();
    size_of_preferential_query=0;

    query->exec(transaction_begin);

    preferential_query_sql = "select game.AppID,game_name,os,game_intro_img,All_reviews,Old_price,New_price,issue_date\n"
                             "FROM `game`\n"
                             "WHERE\n"
                             "CAST(SUBSTRING_INDEX(`new_price`, 'S$', -1) AS DECIMAL(10, 2)) <=\n"
                             ":discount * CAST(SUBSTRING_INDEX(`old_price`, 'S$', -1) AS DECIMAL(10, 2))\n"
                             "ORDER BY\n"
                             "CAST(SUBSTRING_INDEX(`new_price`, 'S$', -1) AS DECIMAL(10, 2)) /\n"
                             "CAST(SUBSTRING_INDEX(`old_price`, 'S$', -1) AS DECIMAL(10, 2)) ASC;";
    query->prepare(preferential_query_sql);
    query->bindValue(":discount",dis);
    query->exec();
    create_information_of_game(7);
    query->exec(transaction_submission);
    return true;
}

Detail_information_of_game Qquery_to_database::get_detail_information_of_game(int appid)
{
    query->exec(transaction_begin);

    query->clear();
    QString detail_information_query_sql = "select AppID,game_name,os,game_intro_img,Recent_reviews,All_reviews,\n"
                                           "Old_price,New_price,issue_date,developer_id,publisher_id from game_details \n"
                                           "where AppID=:appid";
    query->prepare(detail_information_query_sql);
    query->bindValue(":appid",appid);
    query->exec();
    Detail_information_of_game a;
    query->next();

    a.brief.appid=query->value("AppID").toInt();
    a.brief.image_URL=query->value("game_intro_img").toString();
    a.brief.name=query->value("game_name").toString();
    a.brief.support_os=analysis_os(query->value("os").toString());
    a.brief.shelves_time=query->value("issue_date").toDate();
    a.brief.date=a.brief.shelves_time.toString("yyyy-MM-dd");
    analysis_reviews(&(a.brief),query->value("All_reviews").toString());
    a.brief.new_price=query->value("New_price").toString().remove("S$").toDouble();
    a.brief.old_price=query->value("Old_price").toString().remove("S$").toDouble();
    int discount=(a.brief.old_price-a.brief.new_price)*100/a.brief.old_price;
    a.brief.discount="-"+QString::number(discount)+"%";
    a.brief.discount_rate=discount*1.0/100;

    QString positive_rating;
    QString number_of_comments;
    bool digit=true;
    for(auto i:query->value("Recent_reviews").toString())
    {
        if(i>='0'&&i<='9'&&digit)
        {
            positive_rating.append(i);
        }
        else if (i=='%') digit=!digit;
        else if (i>='0'&&i<='9'&&!digit)
        {
            number_of_comments.append(i);
        }
    }
    a.additional.favorable_comparison=positive_rating.toInt()*1.0/100;
    a.additional.number_positive_reviews=number_of_comments.toInt();
    a.additional.number_negative_reviews=(a.additional.number_positive_reviews/a.additional.favorable_comparison)*(1-a.additional.favorable_comparison);
    a.additional.positive_rate=positive_rating+"%";

    a.additional.system_configuration_list=query->value("os").toString().split("#");

    if(a.brief.favorable_comparison>=0.95)     a.additional.evaluate="好评如潮";
    else if(a.brief.favorable_comparison>=0.8) a.additional.evaluate="特别好评";
    else if(a.brief.favorable_comparison>=0.7) a.additional.evaluate="多半好评";
    else if(a.brief.favorable_comparison>=0.4) a.additional.evaluate="褒贬不一";
    else if(a.brief.favorable_comparison>=0.2) a.additional.evaluate="多半差评";
    else if(a.brief.favorable_comparison>=0  ) a.additional.evaluate="差评如潮";

    int publisher_id=query->value("publisher_id").toInt();
    int developer_id=query->value("developer_id").toInt();

    QString get_publisher_name="select developer_name from dev_pub_er where dev_pub_id=:id";
    query->prepare(get_publisher_name);
    query->bindValue(":id",publisher_id);
    query->exec();
    query->next();
    a.additional.publisher=query->value("developer_name").toString();

    query->prepare(get_publisher_name);
    query->bindValue(":id",developer_id);
    query->exec();
    query->next();
    a.additional.developer=query->value("developer_name").toString();

    QString get_game_label = "select tag_name from tag,\n"
                             "(select tag_id from game_to_tag where AppID=:AppID) as a\n"
                             "where a.tag_id=tag.tag_id;";
    query->prepare(get_game_label);
    query->bindValue(":AppID",a.brief.appid);
    query->exec();
    while(query->next())
    {
        a.additional.label_list.push_back(query->value(0).toString());
    }

    query->exec(transaction_submission);
    return a;
}
/*----------------------------------------------------------------------------------------------------------------------------------*/
//排序函数

//日期从晚到前
void Qquery_to_database::date_sort()
{
    if(size_of_label_query != information_game_sequence[0]->size())
    {
        information_game_sequence[0]->clear();
        for(int i=0;i<size_of_label_query;i++) information_game_sequence[0]->push_back(i);
        std::sort((*(information_game_sequence[0])).begin(),(*(information_game_sequence[0])).end(),[this](int dt1,int dt2){
            return brief_information_of_game_list[4][dt1]->shelves_time>brief_information_of_game_list[4][dt2]->shelves_time;
        });
    }
}
//日期从前到晚
void Qquery_to_database::date_desc_sort()
{
    if(size_of_label_query != information_game_sequence[4]->size())
    {
        information_game_sequence[4]->clear();
        for(int i=0;i<size_of_label_query;i++) information_game_sequence[4]->push_back(i);
        std::sort((*(information_game_sequence[4])).begin(),(*(information_game_sequence[4])).end(),[this](int dt1,int dt2){
            return brief_information_of_game_list[4][dt1]->shelves_time<brief_information_of_game_list[4][dt2]->shelves_time;
        });
    }
}
//价格从低到高
void Qquery_to_database::price_sort_ascend()
{
    if(size_of_label_query != information_game_sequence[1]->size())
    {
        information_game_sequence[1]->clear();
        for(int i=0;i<size_of_label_query;i++) information_game_sequence[1]->push_back(i);
        std::sort((*(information_game_sequence[1])).begin(),(*(information_game_sequence[1])).end(),[this](int dt1,int dt2){
            return brief_information_of_game_list[4][dt1]->new_price<brief_information_of_game_list[4][dt2]->new_price;
        });
    }
}
//价格从高到低
void Qquery_to_database::price_sort_descend()
{
    if(size_of_label_query != information_game_sequence[2]->size())
    {
        information_game_sequence[2]->clear();
        for(int i=0;i<size_of_label_query;i++) information_game_sequence[2]->push_back(i);
        std::sort((*(information_game_sequence[2])).begin(),(*(information_game_sequence[2])).end(),[this](int dt1,int dt2){
            return brief_information_of_game_list[4][dt1]->new_price>brief_information_of_game_list[4][dt2]->new_price;
        });
    }
}
//好评率从高到低
void Qquery_to_database::favorable_comparison_sort()
{
    if(size_of_label_query != information_game_sequence[3]->size())
    {
        information_game_sequence[3]->clear();
        for(int i=0;i<size_of_label_query;i++) information_game_sequence[3]->push_back(i);
        std::sort((*(information_game_sequence[3])).begin(),(*(information_game_sequence[3])).end(),[this](int dt1,int dt2){
            return brief_information_of_game_list[4][dt1]->favorable_comparison>brief_information_of_game_list[4][dt2]->favorable_comparison;
        });
    }
}
//好评率从低到高
void Qquery_to_database::favorable_comparison_desc_sort()
{
    if(size_of_label_query != information_game_sequence[5]->size())
    {
        information_game_sequence[5]->clear();
        for(int i=0;i<size_of_label_query;i++) information_game_sequence[5]->push_back(i);
        std::sort((*(information_game_sequence[5])).begin(),(*(information_game_sequence[5])).end(),[this](int dt1,int dt2){
            return brief_information_of_game_list[4][dt1]->favorable_comparison<brief_information_of_game_list[4][dt2]->favorable_comparison;
        });
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------*/

void analysis_reviews(Brief_information_of_game* one,QString Good)
{
    QString positive_rating;
    QString number_of_comments;
    bool digit=true;
    for(auto i:Good)
    {
        if(i>='0'&&i<='9'&&digit)
        {
            positive_rating.append(i);
        }
        else if (i=='%') digit=!digit;
        else if (i>='0'&&i<='9'&&!digit)
        {
            number_of_comments.append(i);
        }
    }
    one->favorable_comparison=positive_rating.toInt()*1.0/100;
    one->number_positive_reviews=number_of_comments.toInt();
    one->number_negative_reviews=(one->number_positive_reviews/one->favorable_comparison)*(1-one->favorable_comparison);
    one->positive_rate=positive_rating+"%";
}
QString analysis_os(QString os)
{
    QString OS="";
    if(os.contains("Windows")) OS+="/Windows";
    if(os.contains("Mac")) OS+="/Mac";
    if(os.contains("Linux")) OS+="/Linux";
    return OS;
}
void Qquery_to_database::create_information_of_game(int index)
{
    Brief_information_of_game* one ;
    while(query->next())
    {
        one = new Brief_information_of_game;
        one->appid=query->value("AppID").toInt();
        one->image_URL=query->value("game_intro_img").toString();
        one->name=query->value("game_name").toString();
        one->support_os=analysis_os(query->value("os").toString());
        one->shelves_time=query->value("issue_date").toDate();
        one->date=one->shelves_time.toString("yyyy-MM-dd");
        analysis_reviews(one,query->value("All_reviews").toString());
        one->new_price=query->value("New_price").toString().remove("S$").toDouble();
        one->old_price=query->value("Old_price").toString().remove("S$").toDouble();
        int discount=(one->old_price-one->new_price)*100/one->old_price;
        one->discount="-"+QString::number(discount)+"%";
        one->discount_rate=discount*1.0/100;
        if(index==1) one->change=query->value("rank_change").toString();
        if(index==3) one->change=query->value("category").toString();
        brief_information_of_game_list[index].push_back(std::move(one));

    }
}

/*----------------------------------------------------------------------------------------------------------------------------------*/
//获取结构体顺序函数

std::vector<int> Qquery_to_database::get_date_sequence()
{
    return (*(information_game_sequence[0]));
}
std::vector<int> Qquery_to_database::get_date_desc_sequence()
{
    return (*(information_game_sequence[4]));
}
std::vector<int> Qquery_to_database::get_price_ascend_sequence()
{
    return (*(information_game_sequence[1]));
}
std::vector<int> Qquery_to_database::get_price_descend_sequence()
{
    return (*(information_game_sequence[2]));
}
std::vector<int> Qquery_to_database::get_favorable_comparison_sequence()
{
    return (*(information_game_sequence[3]));
}
std::vector<int> Qquery_to_database::get_favorable_comparison_desc_sequence()
{
    return (*(information_game_sequence[5]));
}
/*----------------------------------------------------------------------------------------------------------------------------------*/
//获取游戏信息函数

std::vector<Brief_information_of_game> Qquery_to_database::get_main_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[0].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[0])[i]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_date_label_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[4].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[4])[get_date_sequence()[i]]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_date_desc_label_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[4].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[4])[get_date_desc_sequence()[i]]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_price_ascend_label_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[4].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[4])[get_price_ascend_sequence()[i]]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_price_descend_label_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[4].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[4])[get_price_descend_sequence()[i]]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_favorable_comparison_label_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[4].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[4])[get_favorable_comparison_sequence()[i]]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_favorable_comparison_desc_label_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[4].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[4])[get_favorable_comparison_desc_sequence()[i]]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_week_rank_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[1].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[1])[i]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_month_rank_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[2].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[2])[i]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_year_rank_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[3].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[3])[i]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_new_product_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[6].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[6])[i]);
    }
    return a;
}

std::vector<Brief_information_of_game> Qquery_to_database::get_preferential_query_game(int left,int right)
{
    std::vector<Brief_information_of_game> a;
    int size=brief_information_of_game_list[7].size()-1;
    if(size<left) return a;
    else if (size<=right) right=size;
    for(int i=left;i<=right;i++)
    {
        a.push_back(*(brief_information_of_game_list[7])[i]);
    }
    return a;
}

/*----------------------------------------------Qquery_to_database----------------------------------------------*/
