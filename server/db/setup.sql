CREATE DATABASE IF NOT EXISTS arp_poison_db;

USE arp_poison_db;

DROP TABLE IF EXISTS alerts;

CREATE TABLE alerts (
  alertid VARCHAR(18) NOT NULL,
  from_a  VARCHAR(17) NOT NULL,
  to_a    VARCHAR(15) NOT NULL,
  start_t DATETIME    NOT NULL,
  end_t   DATETIME    NOT NULL,
  status  VARCHAR(8)  NOT NULL,
  PRIMARY KEY(alertid)
);

INSERT INTO alerts (alertid, from_a, to_a, start_t, end_t, status)
  VALUES ('A.4@1fl', '192.168.0.10', '192.168.0.22', '2018-02-07 16:36:50', '2018-02-07 16:36:53', 'Active');

SELECT * FROM alerts;
