/*
Navicat MySQL Data Transfer

Source Server         : 开发环境_97
Source Server Version : 50720
Source Host           : 10.168.1.97:3306
Source Database       : robot

Target Server Type    : MYSQL
Target Server Version : 50720
File Encoding         : 65001

Date: 2018-08-13 10:22:16
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `robot_config`
-- ----------------------------
DROP TABLE IF EXISTS `robot_config`;
CREATE TABLE `robot_config` (
  `room_id` varchar(40) DEFAULT NULL COMMENT '场次id',
  `game_id` int(11) NOT NULL COMMENT '游戏id',
  `batch_id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '场次id',
  `robot_count` int(11) unsigned zerofill DEFAULT NULL COMMENT '机器人数量',
  `service_type` tinyint(4) DEFAULT NULL COMMENT '服务模式',
  `entry_time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP COMMENT '进入时间',
  `leave_time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP COMMENT '离开时间',
  `min_coins` bigint(20) DEFAULT NULL COMMENT '携带最小金币',
  `max_coins` bigint(20) DEFAULT NULL COMMENT '携带最大金币',
  `entry_min_interval` int(11) DEFAULT NULL COMMENT '进入最少间隔',
  `entry_max_interval` int(11) DEFAULT NULL COMMENT '进入最大间隔',
  `min_round` int(11) DEFAULT NULL COMMENT '最少陪玩局数',
  `max_round` int(11) DEFAULT NULL COMMENT '最大陪玩局数',
  `min_play_time` int(11) DEFAULT NULL COMMENT '最小出牌时间',
  `max_play_time` int(11) DEFAULT NULL COMMENT '最大出牌时间',
  `min_winning_ratio` smallint(6) DEFAULT NULL COMMENT '最小获胜概率',
  `max_winning_ratio` smallint(6) DEFAULT NULL COMMENT '最大获胜概率',
  `modify_time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP COMMENT '修改时间',
  `modify_user` varchar(50) DEFAULT NULL COMMENT '修改者',
  `status` tinyint(4) DEFAULT '1' COMMENT '是否可用-0：不可用，1：可用',
  PRIMARY KEY (`batch_id`),
  UNIQUE KEY `batch_id` (`batch_id`),
  KEY `game_id` (`game_id`),
  KEY `room_id` (`room_id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Records of robot_config
-- ----------------------------
INSERT INTO `robot_config` VALUES ('3:00:40400100', '40400100', '1', '00000000100', '2', '2018-08-09 15:27:57', '2018-12-31 14:50:47', '10', '1000', '100', '1000', '1', '4', '50', '500', '25', '50', '2018-08-09 15:27:57', 'linus', '1');
INSERT INTO `robot_config` VALUES ('3:01:40400100', '40400100', '2', '00000000200', '1', '2018-08-09 15:28:05', '2018-12-31 14:50:41', '1000', '1000000000', '100', '500', '2', '3', '100', '1000', '20', '75', '2018-08-09 15:28:05', 'linus', '1');
INSERT INTO `robot_config` VALUES ('3:02:40400100', '40400100', '3', '00000000150', '0', '2018-08-09 15:28:51', '2018-12-31 14:50:39', '10000', '1000000000', '50', '2000', '2', '10', '200', '800', '10', '95', '2018-08-09 15:28:51', 'linus', '1');
INSERT INTO `robot_config` VALUES ('3:01:31001000', '31001000', '4', '00000000020', '1', '2018-08-07 10:53:06', '2018-12-31 10:50:34', '10000', '1000000000', '10', '50', '1', '5', '30', '200', '10', '70', '2018-08-07 10:53:06', 'linus', '1');
