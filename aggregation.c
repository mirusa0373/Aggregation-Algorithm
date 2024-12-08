
/* Instruction for this algorithm: When manipulating the light sensor values in the Cooja Simulator, please 
ensure the values do not exceed 150 lux; exceeding 150 lux could lead to integer overflow, leading to 
inaccurate calculations. For easier manipulation, I have scaled down the light sensor values by a factor of 10 (as shown in line 221).  

Algorithm must run within Contiki OS.

Thank you. */


/*------------------------------------------------------------------------------------------------------*/

#include "contiki.h"

#include "dev/light-sensor.h" // For light sensor

#include <stdio.h> // For printf()
#include <random.h> // For random_rand()

#define MAX_VALUES 12 // Maximum number of values for buffer


/* Implementing readable floats: */

int d1(float f) // Integer part
{
	return((int)f);
}

unsigned int d2(float f) // Fractional part
{
	if(f>0)
		return(1000*(f-d1(f)));
	else
		return(1000*(d1(f)-f));
}


/* Function to print array: */
void printArray (float *array, int length) // where (length = number of element in array) - 1
{
	printf("[");
	int i;
	for (i=0; i<length; i++)
	{
		printf("%d.%d, ", d1(array[i]), d2(array[i]));
	}
	printf("%d.%d]", d1(array[i]), d2(array[i]));
}

/* Function to report outcomes: */
void reportOutcome (float *buffer, float *X, int length, float std_dev, char *aggregation) 
// where length = (number of elements in X array) - 1
{
	printf("\n\nB = ");
	printArray(buffer, 11);
					
	printf("\nStdDev = %d.%03u", d1(std_dev), d2(std_dev));
	printf("\nAggregation: %s", aggregation);

	printf("\nX = ");
	printArray(X, length);
	printf("\n");
}

/* Function to obtain and convert light readings into meaningful values: */
float getLight(void)
{
	float V_sensor = 1.5 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC)/4096; // ADC-12 uses 1.5
	float I = V_sensor/100000; // Ohm's law, V = I * R
	float light_lx = 0.625*1e6*I*1000; // Convert current to light intensity, based on data sheet
	return light_lx;
}


/*------------------------------------------------------------------------------------------------------*/

// Declare aggregation process:
PROCESS(aggregation, "Aggregation");
AUTOSTART_PROCESSES(&aggregation);

/*------------------------------------------------------------------------------------------------------*/


/* Implement process: */
PROCESS_THREAD(aggregation, ev, data)
{
	// Static variables (these values are kept between kernel calls):
	static struct etimer timer;
	static int count = 0;
	static float sum = 0, average = 0, difference = 0, variance = 0, std_dev = 0;
	static float buffer[12];

	// Non-static variables (these variables are recomputed at every run
	// so not necessary to declare them as static).
	float light_lx;
	int i;
	char aggregation[100];
	


	// Start of process:
	PROCESS_BEGIN();
	
	// Initialise sensor: 
	SENSORS_ACTIVATE(light_sensor);

	// Initialise variables:
	count = 0;
	sum = 0;
	average = 0;
	difference = 0;
	variance = 0;
	std_dev = 10; // Intial guess for babylonian method of square rooting variance
	buffer[12];

	light_lx = 0;
	i = 0;
	aggregation[100];


	etimer_set(&timer, CLOCK_CONF_SECOND/2); // Generate event in 0.5 second


	while(1)
	{
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);  // Wait here for timer to expire

		if (count==MAX_VALUES)
		{
			// Find the average
			average = sum / MAX_VALUES;

			// Find the variance
			for (i=0; i<12; i++)
			{
				difference = buffer[i] - average;
				variance += (difference * difference) ;  
			}
			variance /= MAX_VALUES;

			// Find the standard deviation (square root of variance)
			// Babylonian method
			float differ = 0.0; 
			float error = 0.001; // Error tolerance
			int j;

			for (j=0; j<50; j++) // up to 50 iterations if no breaking
			{
				std_dev = 0.5 * (std_dev + variance/std_dev);
				differ = std_dev*std_dev - variance;
				if (differ<0) difference = -difference;
				if (differ<error) break; // difference deemed to be too small
			}	
		
			/* Classify levels of activity based on standard deviation: */
			// Low level of activity:
			if (std_dev <= 20) 
			{
				snprintf(aggregation, sizeof(aggregation), "12-into-1");
				float X[1];
				X[0] = average;

				reportOutcome(buffer, X, 0, std_dev, aggregation);

			}

			// Some level of activity:
			if ((std_dev > 20) && (std_dev < 40)) 
			{
				snprintf(aggregation, sizeof(aggregation), "4-into-1");
				float X[3];

				for (i=0; i<3; i++)
				{
					X[i] = (buffer[(4*i)] + buffer[((4*i)+1)] + buffer[((4*i)+2)] 
+ buffer[((4*i)+3)])/4; // (every 4 values are averaged (4n+3, where n = 0,1,2)
				}
				
				reportOutcome(buffer, X, 2, std_dev, aggregation);
			}

			// High level of activity:
			if (std_dev >= 40)
			{
				snprintf(aggregation, sizeof(aggregation), "No aggregation");
				float X[12];
				for (i=0; i<12; i++)
				{
					X[i] = buffer[i];

				}

				reportOutcome(buffer, X, 11, std_dev, aggregation);
			}


			// Reset variables
			count = 0;
			sum = 0;
			average = 0;
			difference = 0;
			variance = 0;
			std_dev = 10;
			buffer[12];
			light_lx = 0;
			i = 0;
			aggregation[100];

		}

		else 
		{
			light_lx = getLight()/10; // Scaled down by factor 10 for easier manipulating of light sensor values
			printf("\nLight Reading %d: %d.%02u", count+1, d1(light_lx), d2(light_lx));
			sum += light_lx;
			buffer[count] = light_lx;
			count++;
		}


		// Reset timer:
		etimer_reset(&timer);


		}
		
		// End of process:
		PROCESS_END();
	}


/*------------------------------------------------------------------------------------------------------*/


