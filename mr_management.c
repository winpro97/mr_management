
#include <stdio.h>
#include <errno.h>
#include <wiringPi.h>

#include <mysql/mysql.h>

//  TMP36 parameters
#define TMP36_BIAS              500.0
#define TMP36_SCALE_FACTOR      100.0

#define CS_MCP3208  10

#define SPI_CHANNEL0 0
#define SPI_CHANNEL1 0
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

  wiringPiSPIDataRW(adcChannel, buff, 3);

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

//InfraRedRay
int isPerson(unsigned char adcChannel) {
    int              val;
	val = read_mcp3208_adc(SPI_CHANNEL1);
	if(val>1000) {
		return 1;
	} else {
		return 0;
	}
}

void printTemperature(unsigned char adcChannel) {
    int              val;
    double           dlsb;
    double           degC;
    double           degF;
    double           fVal;
    double           fdegC;
    double           fdegF;
    short            bInit = 1;
	
	//  define the parameters we will use to perform conversions
    //  this is the MCP3002 which has power of 3.3V and 10 bit A/D
    //  so value of the lsb is voltage range / maximum value
    dlsb = 3300 / 1023.0;

    //  loop forever polling the temperature
    //for (;;)
    //{
        val = read_mcp3208_adc(SPI_CHANNEL1);
        degC = ((double) val * dlsb - TMP36_BIAS) / TMP36_SCALE_FACTOR;
        degF = degC * 9.0 / 5.0 + 32.0;

        //  implement a leaky bucket integrator to smooth the data, although
        //  for this sensore it already appears quite stable. If the initialize
        //  flag is set, initialize the filtered value to the input value and
        //  clear the initialize flag
        if (bInit)
        {
            fVal = (double) val;
            bInit = 0;
        }

        //  for this leaky bucket integrator we have chosen 0.875 * filtered value +
        //  0.125 * raw value
        fVal = 7.0 * fVal / 8.0 + (double) val / 8.0;
        fdegC = (fVal * dlsb - TMP36_BIAS) / TMP36_SCALE_FACTOR;
        fdegF = fdegC  * 9.0 / 5.0 + 32.0;
        printf("Tmp36 - Value: %d, Deg C: %lf, Deg F: %lf, Filtered Deg C: %lf, Deg F: %lf\n",
               val, degC, degF, fdegC, fdegF);

        //  sleep until next sample time
        //sleep(5);
    //}
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
  
  if(wiringPiSPISetup(SPI_CHANNEL0, SPI_SPEED) == -1 || wiringPiSPISetup(SPI_CHANNEL1, SPI_SPEED) == -1)
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
