
#include <stdio.h>
#include <errno.h>
#include <wiringPi.h>

#include <mysql/mysql.h>

#define CS_MCP3208  10

#define SPI_CHANNEL 0
#define SPI_SPEED   1000000   // 1MHz

MYSQL *connector;

#define DBHOST "127.0.0.1"
#define DBUSER "root"
#define DBPASS "raspberry"
#define DBNAME "rpidb"

int read_mcp3208_adc(unsigned char adcChannel)
{
  unsigned char buff[3];
  int adcValue = 0;

  buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
  buff[1] = ((adcChannel & 0x07) << 6);
  buff[2] = 0x00;

  digitalWrite(CS_MCP3208, 0);  // Low : CS Active

  wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);

  buff[1] = 0x0F & buff[1];
  adcValue = ( buff[1] << 8) | buff[2];

  digitalWrite(CS_MCP3208, 1);  // High : CS Inactive

  return adcValue;
}

void writeToDB(int illuminance)
{
    char query[1024];

    //sprintf(query,"insert into envdata (id, motion, time) values (1, %d, now())", motion);
    sprintf(query,"update envdata set illuminance = %d, time = now()", illuminance);

    if(mysql_query(connector, query))
    {
      fprintf(stderr, "%s\n", mysql_error(connector));
      printf("Write DB error\n");
    }
}

int main (void)
{
	int gas = 0;
	int motion = 0;
	int illuminance = 0;
	int flame = 0;
	
	pthread_t tId;
	
  if(wiringPiSetup() == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror(errno));
    return 1 ;
  }
  
  if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
  {
    fprintf (stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));
    return 1 ;
  }
  
  pinMode(CS_MCP3208, OUTPUT);
  
  // MySQL connection
  connector = mysql_init(NULL);
  if (!mysql_real_connect(connector, DBHOST, DBUSER, DBPASS, DBNAME, 3306, NULL, 0))
  {
    printf("%s\n", mysql_error(connector));
    return 0;
  }

  while(1)
  {
  	illuminance = read_mcp3208_adc(0); // Illuminance Sensor
    
  	writeToDB(illuminance);
  	
  	delay(1000);
  }
  
  mysql_close(connector);
}
