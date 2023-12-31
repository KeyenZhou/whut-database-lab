-- MySQL Script generated by MySQL Workbench
-- Sat Dec 30 19:44:42 2023
-- Model: New Model    Version: 1.0
-- MySQL Workbench Forward Engineering

SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';

-- -----------------------------------------------------
-- Schema Steam
-- -----------------------------------------------------
-- Steam游戏数据库
DROP SCHEMA IF EXISTS `Steam` ;

-- -----------------------------------------------------
-- Schema Steam
--
-- Steam游戏数据库
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `Steam` DEFAULT CHARACTER SET utf8mb4 ;
USE `Steam` ;

-- -----------------------------------------------------
-- Table `content`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `content` ;

CREATE TABLE IF NOT EXISTS `content` (
  `content_id` INT NOT NULL AUTO_INCREMENT,
  `content_content` TEXT NOT NULL,
  `content_date` TEXT NOT NULL,
  PRIMARY KEY (`content_id`))
ENGINE = InnoDB
COMMENT = '评论';

CREATE UNIQUE INDEX `content_id_UNIQUE` ON `content` (`content_id` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `content_to_game`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `content_to_game` ;

CREATE TABLE IF NOT EXISTS `content_to_game` (
  `content_id` INT NOT NULL,
  `AppID` INT NOT NULL,
  PRIMARY KEY (`content_id`, `AppID`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `content_to_user`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `content_to_user` ;

CREATE TABLE IF NOT EXISTS `content_to_user` (
  `user_id` INT NOT NULL,
  `content_id` INT NOT NULL,
  PRIMARY KEY (`user_id`, `content_id`))
ENGINE = InnoDB;

CREATE UNIQUE INDEX `user_id_UNIQUE` ON `content_to_user` (`user_id` ASC) VISIBLE;

CREATE UNIQUE INDEX `content_id_UNIQUE` ON `content_to_user` (`content_id` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `dev_pub_er`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `dev_pub_er` ;

CREATE TABLE IF NOT EXISTS `dev_pub_er` (
  `dev_pub_id` INT NOT NULL AUTO_INCREMENT,
  `developer_name` VARCHAR(45) NOT NULL,
  `about` TEXT NULL,
  `img` TEXT NULL,
  PRIMARY KEY (`dev_pub_id`))
ENGINE = InnoDB;

CREATE UNIQUE INDEX `developer_id_UNIQUE` ON `dev_pub_er` (`dev_pub_id` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `developer_to_game`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `developer_to_game` ;

CREATE TABLE IF NOT EXISTS `developer_to_game` (
  `developer_id` INT NOT NULL,
  `AppID` INT NOT NULL,
  PRIMARY KEY (`developer_id`, `AppID`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `game`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `game` ;

CREATE TABLE IF NOT EXISTS `game` (
  `AppID` INT NOT NULL COMMENT '游戏唯一标识符',
  `game_name` VARCHAR(100) NOT NULL DEFAULT '无',
  `os` TEXT NOT NULL,
  `game_intro_img` TEXT NULL,
  `Recent_reviews` TEXT NOT NULL COMMENT '最近三十天',
  `All_reviews` TEXT NOT NULL COMMENT '全部',
  `Old_price` VARCHAR(20) NOT NULL DEFAULT '0',
  `New_price` VARCHAR(20) NOT NULL DEFAULT '0',
  `issue_date` DATE NOT NULL DEFAULT '2023-01-01',
  `developer_id` INT NOT NULL DEFAULT -1,
  `publisher_id` INT NOT NULL DEFAULT -1,
  `language` TEXT NULL,
  PRIMARY KEY (`AppID`))
ENGINE = InnoDB;

CREATE UNIQUE INDEX `AppID_UNIQUE` ON `game` (`AppID` ASC) VISIBLE;

CREATE UNIQUE INDEX `game_name_UNIQUE` ON `game` (`game_name` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `game_library`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `game_library` ;

CREATE TABLE IF NOT EXISTS `game_library` (
  `AppID` INT NOT NULL,
  `game_name` VARCHAR(100) NOT NULL,
  PRIMARY KEY (`AppID`))
ENGINE = InnoDB;

CREATE UNIQUE INDEX `AppID_UNIQUE` ON `game_library` (`AppID` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `game_to_tag`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `game_to_tag` ;

CREATE TABLE IF NOT EXISTS `game_to_tag` (
  `AppID` INT NOT NULL,
  `tag_id` INT NOT NULL,
  PRIMARY KEY (`AppID`, `tag_id`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `month_rank`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `month_rank` ;

CREATE TABLE IF NOT EXISTS `month_rank` (
  `Date` DATE NOT NULL,
  `AppID` INT NOT NULL,
  PRIMARY KEY (`Date`, `AppID`))
ENGINE = InnoDB
COMMENT = '月新品前20';

CREATE INDEX `Date` ON `month_rank` (`Date` ASC) INVISIBLE;

CREATE INDEX `AppID` ON `month_rank` (`AppID` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `query_record`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `query_record` ;

CREATE TABLE IF NOT EXISTS `query_record` (
  `record_ID` INT NOT NULL AUTO_INCREMENT,
  `query_time` DATE NOT NULL,
  `query_content` TEXT NOT NULL COMMENT '不同标签用\'/\'连接',
  PRIMARY KEY (`record_ID`))
ENGINE = InnoDB;

CREATE UNIQUE INDEX `record_ID_UNIQUE` ON `query_record` (`record_ID` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `tag`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `tag` ;

CREATE TABLE IF NOT EXISTS `tag` (
  `tag_id` INT NOT NULL AUTO_INCREMENT COMMENT 'tag唯一标识符',
  `tag_name` VARCHAR(45) NOT NULL COMMENT '标签名',
  `tag_type` TINYINT NOT NULL DEFAULT 0 COMMENT '标签类型 0-系统的 1-普通的',
  `user_id` INT NOT NULL DEFAULT 1,
  `tag_times` INT NOT NULL DEFAULT 0,
  PRIMARY KEY (`tag_id`))
ENGINE = InnoDB;

CREATE UNIQUE INDEX `tag_id_UNIQUE` ON `tag` (`tag_id` ASC) VISIBLE;

CREATE UNIQUE INDEX `tag_name_UNIQUE` ON `tag` (`tag_name` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `user`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `user` ;

CREATE TABLE IF NOT EXISTS `user` (
  `user_id` INT NOT NULL AUTO_INCREMENT COMMENT '用户唯一标识符',
  `user_account` VARCHAR(20) NOT NULL COMMENT '用户账号',
  `user_password` VARCHAR(20) NOT NULL COMMENT '用户密码',
  `user_gender` VARCHAR(10) NOT NULL DEFAULT '不显示' COMMENT '用户性别',
  `user_email` VARCHAR(45) NULL COMMENT '用户邮箱',
  `user_status` INT NOT NULL DEFAULT 0 COMMENT '用户状态 0-不在线  1-在线  -1-注销',
  `user_admin` TINYINT NOT NULL DEFAULT 0 COMMENT '0-普通用户\n1-系统管理员',
  PRIMARY KEY (`user_id`))
ENGINE = InnoDB;

CREATE UNIQUE INDEX `user_id_UNIQUE` ON `user` (`user_id` ASC) VISIBLE;

CREATE UNIQUE INDEX `user_account_UNIQUE` ON `user` (`user_account` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `user_to_query`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `user_to_query` ;

CREATE TABLE IF NOT EXISTS `user_to_query` (
  `record_ID` INT NOT NULL,
  `user_id` INT NOT NULL,
  PRIMARY KEY (`record_ID`, `user_id`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `user_to_tag`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `user_to_tag` ;

CREATE TABLE IF NOT EXISTS `user_to_tag` (
  `user_id` INT NOT NULL,
  `tag_id` INT NOT NULL,
  PRIMARY KEY (`tag_id`, `user_id`))
ENGINE = InnoDB
COMMENT = '联合表';


-- -----------------------------------------------------
-- Table `week_rank`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `week_rank` ;

CREATE TABLE IF NOT EXISTS `week_rank` (
  `Date` DATE NOT NULL,
  `AppID` INT NOT NULL,
  `rank` INT NOT NULL,
  PRIMARY KEY (`Date`, `AppID`, `rank`))
ENGINE = InnoDB;

CREATE INDEX `Date` ON `week_rank` (`Date` ASC) VISIBLE;


-- -----------------------------------------------------
-- Table `year_rank`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `year_rank` ;

CREATE TABLE IF NOT EXISTS `year_rank` (
  `Date` DATE NOT NULL,
  `AppID` INT NOT NULL,
  `category` VARCHAR(45) NOT NULL DEFAULT '无',
  PRIMARY KEY (`Date`, `AppID`))
ENGINE = InnoDB;

CREATE INDEX `Date` ON `year_rank` (`Date` ASC) INVISIBLE;

CREATE INDEX `AppID` ON `year_rank` (`AppID` ASC) INVISIBLE;

CREATE INDEX `category` ON `year_rank` (`category` ASC) VISIBLE;

USE `Steam`;

DELIMITER $$

USE `Steam`$$
DROP TRIGGER IF EXISTS `user_BEFORE_DELETE` $$
USE `Steam`$$
CREATE DEFINER = CURRENT_USER TRIGGER `Steam`.`user_BEFORE_DELETE` BEFORE DELETE ON `user` FOR EACH ROW
BEGIN
    CREATE TEMPORARY TABLE record_temp_table
    (
        record_ID INT
    );
    insert into record_temp_table
    SELECT record_ID
    FROM user_to_query
    WHERE user_id = OLD.user_id;


    DELETE FROM user_to_query WHERE user_id = OLD.user_id;

    DELETE FROM query_record WHERE record_ID in (select record_ID from record_temp_table);
    DROP TEMPORARY TABLE IF EXISTS record_temp_table;
END$$


USE `Steam`$$
DROP TRIGGER IF EXISTS `user_BEFORE_DELETE_1` $$
USE `Steam`$$
CREATE DEFINER = CURRENT_USER TRIGGER `Steam`.`user_BEFORE_DELETE_1` BEFORE DELETE ON `user` FOR EACH ROW
BEGIN
    CREATE TEMPORARY TABLE tag_temp_table
    (
        tag_id INT
    );
    insert into tag_temp_table
    SELECT tag_id
    FROM user_to_tag
    WHERE user_id = OLD.user_id;


    DELETE FROM user_to_tag WHERE user_id = OLD.user_id;

    DELETE FROM tag WHERE tag_id in (select tag_id from tag_temp_table);
    DROP TEMPORARY TABLE IF EXISTS tag_temp_table;
END$$


USE `Steam`$$
DROP TRIGGER IF EXISTS `user_BEFORE_DELETE_2` $$
USE `Steam`$$
CREATE DEFINER = CURRENT_USER TRIGGER `Steam`.`user_BEFORE_DELETE_2` BEFORE DELETE ON `user` FOR EACH ROW
BEGIN
    CREATE TEMPORARY TABLE content_temp_table
    (
        content_id INT
    );
    insert into content_temp_table
    SELECT content_id
    FROM content_to_user
    WHERE user_id = OLD.user_id;


    DELETE FROM content_to_user WHERE user_id = OLD.user_id;

    DELETE FROM content WHERE content_id in (select content_id from content_temp_table);
    DROP TEMPORARY TABLE IF EXISTS content_temp_table;
END$$


DELIMITER ;

SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
