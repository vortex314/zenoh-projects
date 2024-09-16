#include <Log.h>
#include <Sema.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>


class Esp32Semaphore : public Sema {
		SemaphoreHandle_t xSemaphore = NULL;
		uint32_t counter=0;
	public:
		Esp32Semaphore() {
			xSemaphore = xSemaphoreCreateBinary();
			xSemaphoreGive(xSemaphore);
		}
		~Esp32Semaphore() {}

		void wait() {
			if (xSemaphoreTake(xSemaphore, (TickType_t)100000) == pdFALSE) {
				WARN(" xSemaphoreTake()  timed out ");
			}
			counter++;
			if ( counter != 1 ) {
				WARN(" =======> wait sema counter %d \n",counter);
			}
		}

		void release() {
			counter--;
			if ( counter != 0 ) {
				WARN(" ======> give sema counter %d \n",counter);
			}
			if (xSemaphoreGive(xSemaphore) == pdFALSE) {
				WARN("xSemaphoreGive() failed");
			}


		}
//   static Semaphore& create() { return *(new Esp32Semaphore()); }
};

Sema& Sema::create() {return *new Esp32Semaphore();}

