#include <Wire.h>
#include <SPI.h>

#include <Adafruit_BMP085.h>
#include <Adafruit_BMP280.h>

#define SENSOR_INTERVAL 100
#define SAMPLES_COUNT 10

#define BMP_SCK  13  // SPI Clock
#define BMP_MISO 12  // SPI MISO
#define BMP_MOSI 11  // SPI MOSI
#define BMP_CS   10  // Chip Select

Adafruit_BMP085 bmp180;
Adafruit_BMP280 bmp280(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);

float bmp180PressureSamples[SAMPLES_COUNT];
float bmp280PressureSamples[SAMPLES_COUNT];

uint8_t sampleIndex = 0;

unsigned long lastSensorRead = 0;

bool bmp180Available = false;
bool bmp280Available = false;

bool isValidPressure(float pressure)
{
	return !isnan(pressure) &&
				 pressure >= 30000.0 &&
				 pressure <= 120000.0;
}

float calculateAverage(float *samples)
{
	float sum = 0.0;

	for (uint8_t i = 0; i < SAMPLES_COUNT; i++)
	{
		sum += samples[i];
	}

	return sum / SAMPLES_COUNT;
}

void setup()
{
	Serial.begin(115200);

	Wire.begin();
	SPI.begin();

	Serial.println();
	Serial.println("======================================");
	Serial.println(" Pressure Monitoring System");
	Serial.println("======================================");

	Serial.print("Initializing BMP180... ");

	if (bmp180.begin())
	{
		bmp180Available = true;
		Serial.println("OK");
	}
	else
	{
		Serial.println("FAILED");
		Serial.println("WARNING: BMP180 is unavailable!");
	}

	Serial.print("Initializing BMP280... ");

	if (bmp280.begin())
	{
		bmp280Available = true;

		bmp280.setSampling(
				Adafruit_BMP280::MODE_NORMAL,
				Adafruit_BMP280::SAMPLING_X1,
				Adafruit_BMP280::SAMPLING_X16,
				Adafruit_BMP280::FILTER_X16,
				Adafruit_BMP280::STANDBY_MS_125);

		Serial.println("OK");
	}
	else
	{
		Serial.println("FAILED");
		Serial.println("WARNING: BMP280 is unavailable!");
	}

	Serial.println();
	Serial.println("Starting measurements...");
	Serial.println();

	lastSensorRead = millis();
}

void loop()
{
	unsigned long currentTime = millis();

	if (currentTime - lastSensorRead >= SENSOR_INTERVAL)
	{
		lastSensorRead += SENSOR_INTERVAL;

		if (bmp180Available)
		{
			float pressure = bmp180.readPressure();

			if (isValidPressure(pressure))
			{
				bmp180PressureSamples[sampleIndex] = pressure;
			}
			else
			{
				bmp180PressureSamples[sampleIndex] = NAN;
			}
		}

		if (bmp280Available)
		{
			float pressure = bmp280.readPressure();

			if (isValidPressure(pressure))
			{
				bmp280PressureSamples[sampleIndex] = pressure;
			}
			else
			{
				bmp280PressureSamples[sampleIndex] = NAN;
			}
		}

		sampleIndex++;

		if (sampleIndex >= SAMPLES_COUNT)
		{
			sampleIndex = 0;

			printResults();
		}
	}
}

void printResults()
{
	Serial.println("--------------------------------------");
	Serial.println("New measurement:");
	Serial.println();

	Serial.print("BMP180 (I2C): ");

	if (!bmp180Available)
	{
		Serial.println("UNAVAILABLE");
	}
	else
	{
		bool valid = true;

		for (uint8_t i = 0; i < SAMPLES_COUNT; i++)
		{
			if (isnan(bmp180PressureSamples[i]))
			{
				valid = false;
				break;
			}
		}

		if (!valid)
		{
			Serial.println("WARNING: INVALID DATA");
		}
		else
		{
			float average = calculateAverage(bmp180PressureSamples);

			Serial.print(average / 100.0, 2);
			Serial.println(" hPa");
		}
	}

	Serial.print("BMP280 (SPI): ");

	if (!bmp280Available)
	{
		Serial.println("UNAVAILABLE");
	}
	else
	{
		bool valid = true;

		for (uint8_t i = 0; i < SAMPLES_COUNT; i++)
		{
			if (isnan(bmp280PressureSamples[i]))
			{
				valid = false;
				break;
			}
		}

		if (!valid)
		{
			Serial.println("WARNING: INVALID DATA");
		}
		else
		{
			float average = calculateAverage(bmp280PressureSamples);

			Serial.print(average / 100.0, 2);
			Serial.println(" hPa");
		}
	}

	if (bmp180Available && bmp280Available)
	{
		bool bmp180Valid = true;
		bool bmp280Valid = true;

		for (uint8_t i = 0; i < SAMPLES_COUNT; i++)
		{
			if (isnan(bmp180PressureSamples[i]))
			{
				bmp180Valid = false;
			}

			if (isnan(bmp280PressureSamples[i]))
			{
				bmp280Valid = false;
			}
		}

		if (bmp180Valid && bmp280Valid)
		{
			float bmp180Average = calculateAverage(bmp180PressureSamples);
			float bmp280Average = calculateAverage(bmp280PressureSamples);
			float difference = abs(bmp180Average - bmp280Average);

			Serial.print("Difference: ");
			Serial.print(difference / 100.0, 2);
			Serial.println(" hPa");

			if (difference > 500.0)
			{
				Serial.println("WARNING: Large pressure difference!");
			}
		}
	}

	Serial.println();
}