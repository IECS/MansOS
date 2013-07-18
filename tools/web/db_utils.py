from settings import *
from uuid import getnode as get_mac
import datetime

connection = None
engine = None
try:
    from sqlalchemy import create_engine
    from sqlalchemy import Table, Column, MetaData
    from sqlalchemy.types import DateTime, Numeric, Integer, String

    # Database
    url = settingsInstance.getCfgValue("dbServer")
    username = settingsInstance.getCfgValue("dbUsername")
    password = settingsInstance.getCfgValue("dbPassword")
    host = settingsInstance.getCfgValue("dbHost")
    engine = create_engine(url % (username, password, host))
    metadata = MetaData()
    observations = Table('observations', metadata,
        Column('id', Integer, primary_key = True),
        Column('obs_time', DateTime),
        Column('unit_id', String),
        Column('port', String),
        Column('type', String),
        Column('value', Numeric(20,6))
    )
except ImportError:
    # TODO: do not print this if "silent" mode is configured in config file!
    print("Warning: using a database for data storage is not possible, package dependencies are missing.")
    print("To store to use a database, install: python-sqlalchemy python-mysqldb\n")

def openDBConnection():
    global connection
    if engine != None:
        connection = engine.connect()

def closeDBConnection():           
    global connection
    if connection != None:
        connection.close()

def saveDataToDB(packet):
    global connection

    mac = str(get_mac())
    for key in packet.keys():
        arr = key.split(":")
        val = packet[key]
        ins = observations.insert().values(obs_time = datetime.datetime.now(), unit_id = mac, port = arr[0], type = arr[1], value = val)
        connection.execute(ins)
