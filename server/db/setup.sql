CREATE DATABASE IF NOT EXISTS arp_poison_db;

USE arp_poison_db;

DROP TABLE IF EXISTS alerts;
DROP TABLE IF EXISTS logs;
DROP TABLE IF EXISTS users;

CREATE TABLE alerts (
  alertid VARCHAR(18) NOT NULL,
  from_a  VARCHAR(17) NOT NULL,
  to_a    VARCHAR(15) NOT NULL,
  start_t DATETIME    NOT NULL,
  end_t   DATETIME    NOT NULL,
  status  VARCHAR(8)  NOT NULL,
  PRIMARY KEY(alertid)
);

CREATE TABLE logs (
  logid  INT AUTO_INCREMENT,
  from_a VARCHAR(17) NOT NULL,
  to_a   VARCHAR(15) NOT NULL,
  time_s DATETIME    NOT NULL,
  PRIMARY KEY(logid)
);

CREATE TABLE users (
  userid   INT         AUTO_INCREMENT,
  username VARCHAR(30) NOT NULL,
  password VARCHAR(30) NOT NULL,
  PRIMARY KEY(userid)
);

INSERT INTO alerts (alertid, from_a, to_a, start_t, end_t, status)
  VALUES ('A.4@1fl', '192.168.0.10', '192.168.0.22', '2018-02-07 16:36:50', '2018-02-07 16:36:53', 'Active');

INSERT INTO alerts (alertid, from_a, to_a, start_t, end_t, status)
  VALUES ('B.4@1fl', '192.168.0.10', '192.168.0.21', '2018-02-06 13:36:16', '2018-02-06 13:36:18', 'Inactive');

INSERT INTO alerts (alertid, from_a, to_a, start_t, end_t, status)
  VALUES ('C.4@1fl', '192.168.0.10', '192.168.0.22', '2018-02-06 05:08:11', '2018-02-06 05:08:12', 'Inactive');


INSERT INTO logs (from_a, to_a, time_s)
  VALUES ('192.168.0.10', '192.168.0.22', '2018-02-07 16:36:50');
INSERT INTO logs (from_a, to_a, time_s)
  VALUES ('192.168.0.10', '192.168.0.22', '2018-02-07 16:36:51');
INSERT INTO logs (from_a, to_a, time_s)
  VALUES ('192.168.0.10', '192.168.0.22', '2018-02-07 16:36:52');
INSERT INTO logs (from_a, to_a, time_s)
  VALUES ('192.168.0.10', '192.168.0.22', '2018-02-07 16:36:53');

INSERT INTO logs (from_a, to_a, time_s)
  VALUES ('192.168.0.10', '192.168.0.21', '2018-02-06 13:36:16');
INSERT INTO logs (from_a, to_a, time_s)
  VALUES ('192.168.0.10', '192.168.0.21', '2018-02-06 13:36:17');
INSERT INTO logs (from_a, to_a, time_s)
  VALUES ('192.168.0.10', '192.168.0.21', '2018-02-06 13:36:18');

INSERT INTO logs (from_a, to_a, time_s)
  VALUES ('192.168.0.11', '192.168.0.22', '2018-02-06 05:08:11');
INSERT INTO logs (from_a, to_a, time_s)
  VALUES ('192.168.0.11', '192.168.0.22', '2018-02-06 05:08:12');

INSERT INTO users (username, password)
  VALUES ('a@a.a', 'P@$$w0rd');

SELECT * FROM alerts;
SELECT * FROM logs;
SELECT * FROM users;
