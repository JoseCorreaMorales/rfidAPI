# RFID REST API

The RFID REST API is a system that leverages various technologies, including PHP, MySQL, and Arduino, to manage data related to RFID cards and RFID sensors. The detailed description is as follows:

<p align="center">
  <img src="https://img.shields.io/badge/-PHP-777BB4?style=for-the-badge&logo=php&logoColor=white" alt="PHP Icon" />
  <img src="https://img.shields.io/badge/-REST%20API-FF6C37?style=for-the-badge" alt="REST API Icon" />
  <img src="https://img.shields.io/badge/-MySQL-4479A1?style=for-the-badge&logo=mysql&logoColor=white" alt="MySQL Icon" />
  <img src="https://img.shields.io/badge/-RFID%20Cards-009688?style=for-the-badge" alt="RFID Cards Icon" />
  <img src="https://img.shields.io/badge/-RFID%20Sensors-009688?style=for-the-badge" alt="RFID Sensors Icon" />
  <img src="https://img.shields.io/badge/-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white" alt="Arduino Icon" />
</p>

# Arduino

rest.ino is an Arduino sketch that utilizes an ESP32 microcontroller to communicate with a REST API. It includes functionality for sending HTTP requests with an authorization header, parsing JSON responses, and displaying information on an LCD screen.

## Usage
To use rest.ino, an ESP32 microcontroller and an LCD screen need to be connected. Also, a REST API that accepts HTTP requests with an authorization header and returns JSON responses is required.

Once the hardware is set up and the REST API is configured, upload rest.ino to the ESP32 using the Arduino IDE. The sketch will run automatically when the ESP32 is powered on.

When the sketch runs, it reads RFID tag data from a connected reader and sends an HTTP POST request to the REST API with the tag ID. If the request is successful, the sketch parses the JSON response and displays information about the transaction on the LCD screen.

## RFIDBD

RFIDBD is an SQL database designed for managing RFID-related data. It includes tables for users, movements, and balances, and can be used to track RFID tags and their associated transactions.



<p align="center">
<img src="placa/ER diagram.png" alt="ER DIAGRAM" width="550"/>
</p>

## Installation
To install RFIDBD, a SQL server needs to be installed on localhost. Then, run the rfidbd.sql script to create the necessary tables and constraints.

## Usage
Once the database is set up, it can be used to store and retrieve RFID data. The users table includes information about each user, including their ID and name. The movements table includes information about each transaction, including the user ID and transaction amount. The balances table includes information about the balance of each user.

To use the database, write SQL queries to insert, update, and retrieve data. For example, to insert a new user, run the following query:

```sql
INSERT INTO users (name) VALUES ('John Doe');
```
To retrieve the balance of a user, run the following query:
```sql
SELECT balance FROM balances WHERE userid = 1;
```
