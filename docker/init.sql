CREATE DATABASE IF NOT EXISTS packet_capture_db;
USE packet_capture_db;

CREATE TABLE IF NOT EXISTS capture_sessions (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    interface_name VARCHAR(128) NOT NULL,
    filter_expression VARCHAR(255) NOT NULL DEFAULT '',
    started_at DATETIME NOT NULL,
    ended_at DATETIME NULL,
    packet_count BIGINT NOT NULL DEFAULT 0,
    active TINYINT(1) NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS packets (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    session_id BIGINT NOT NULL,
    packet_number BIGINT NOT NULL,
    time_text VARCHAR(64) NOT NULL,
    source_ip VARCHAR(64) NOT NULL,
    destination_ip VARCHAR(64) NOT NULL,
    protocol VARCHAR(30) NOT NULL,
    length INT NOT NULL,
    info_text TEXT,
    captured_at DATETIME NOT NULL,
    INDEX idx_packets_session_id (session_id),
    CONSTRAINT fk_packets_session_id FOREIGN KEY (session_id) REFERENCES capture_sessions(id) ON DELETE CASCADE
);
