/*
 * thing_4_inputs.c
 *
 *  Created on: Apr 19, 2021
 *      Author: Krzysztof Zurek
 *		e-mail: krzzurek@gmail.com
 */
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "simple_web_thing_server.h"
#include "thing_4_inputs.h"

//button GPIO
#define GPIO_INPUT_1	    (CONFIG_INPUT_1_GPIO)
#define GPIO_INPUT_2	    (CONFIG_INPUT_2_GPIO)
#define GPIO_INPUT_3	    (CONFIG_INPUT_3_GPIO)
#define GPIO_INPUT_4	    (CONFIG_INPUT_4_GPIO)
#define GPIO_INPUT_MASK		(1ULL << GPIO_INPUT_1)|(1ULL << GPIO_INPUT_2)|\
							(1ULL << GPIO_INPUT_3)|(1ULL << GPIO_INPUT_4)

#define IN_1 0x01
#define IN_2 0x02
#define IN_3 0x04
#define IN_4 0x08

#define CNT_MAX 1000

xSemaphoreHandle DRAM_ATTR input_sem;
xSemaphoreHandle DRAM_ATTR inputs_mux;
static uint8_t DRAM_ATTR inputs = 0;
static int32_t DRAM_ATTR input_counter = 0;

thing_t *iot_4_inputs = NULL;
property_t *prop_input_1, *prop_input_2, *prop_input_3, *prop_input_4;
property_t *prop_input_counter, *prop_in_on_off;
at_type_t iot_4_inputs_type, input_1_prop_type, input_3_prop_type;
at_type_t input_4_prop_type;
at_type_t input_counter_prop_type, on_off_prop_type;
static bool in_1, in_2, in_3, in_4;
static bool on_off_state = false;

/* ************************************************************
 *
 * input-1 interrupt
 *
 * ***********************************************************/
static void IRAM_ATTR input_1_isr_handler(void* arg){
	static portBASE_TYPE xHigherPriorityTaskWoken;

	inputs |= IN_1;
	xSemaphoreGiveFromISR(input_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR();
}


/* ************************************************************
 *
 * input-2 interrupt
 *
 * ***********************************************************/
static void IRAM_ATTR input_2_isr_handler(void* arg){
	static portBASE_TYPE xHigherPriorityTaskWoken;

	inputs |= IN_2;
	xSemaphoreGiveFromISR(input_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR();
}


/* ************************************************************
 *
 * input-3 interrupt
 *
 * ***********************************************************/
static void IRAM_ATTR input_3_isr_handler(void* arg){
	static portBASE_TYPE xHigherPriorityTaskWoken;

	inputs |= IN_3;
	xSemaphoreGiveFromISR(input_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR();
}


/* ************************************************************
 *
 * input-4 interrupt
 *
 * ***********************************************************/
static void IRAM_ATTR input_4_isr_handler(void* arg){
	static portBASE_TYPE xHigherPriorityTaskWoken;

	inputs |= IN_4;
	xSemaphoreGiveFromISR(input_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR();
}


/*************************************************************
 *
 * main button function
 *
 * ************************************************************/
void inputs_fun(void *pvParameter){
	int input_1 = 0, input_2 = 0, input_3 = 0, input_4 = 0;
	uint8_t inform = 0;

	printf("Inputs task is ready\n");

	for(;;){
		//wait for button pressed
		xSemaphoreTake(input_sem, portMAX_DELAY);
		inform = 0;
		input_1 = gpio_get_level(GPIO_INPUT_1);
		input_2 = gpio_get_level(GPIO_INPUT_2);
		input_3 = gpio_get_level(GPIO_INPUT_3);
		input_4 = gpio_get_level(GPIO_INPUT_4);
		if (inputs & IN_1){
			inputs &= ~IN_1;
			//get pin value
			//printf("%i, IN-1: %i, %i\n", input_counter, input_1, in_1);
			if (input_1 != in_1){
				inform |= IN_1;
				in_1 = input_1;
				input_counter++;
			}
		}
		if (inputs & IN_2){
			inputs &= ~IN_2;
			//get pin value
			//printf("%i, IN-2: %i, %i\n", input_counter, input_2, in_2);
			if (input_2 != in_2){
				inform |= IN_2;
				in_2 = input_2;
				input_counter++;
			}
		}
		if (inputs & IN_3){
			inputs &= ~IN_3;
			//get pin value
			//printf("%i, IN-3: %i, %i\n", input_counter, input_3, in_3);
			if (input_3 != in_3){
				inform |= IN_3;
				in_3 = input_3;
				input_counter++;
			}
		}
		if (inputs & IN_4){
			inputs &= ~IN_4;
			//get pin value
			//printf("%i, IN-4: %i, %i\n", input_counter, input_4, in_4);
			if (input_4 != in_4){
				inform |= IN_4;
				in_4 = input_4;
				input_counter++;
			}
		}
		if (input_counter > CNT_MAX){
			input_counter = 0;
		}

		if (inform & IN_1){
			inform_all_subscribers_prop(prop_input_1);
		}
		if (inform & IN_2){
			inform_all_subscribers_prop(prop_input_2);
		}
		if (inform & IN_3){
			inform_all_subscribers_prop(prop_input_3);
		}
		if (inform & IN_4){
			inform_all_subscribers_prop(prop_input_4);
		}
		if (input_counter%10 == 0){
			inform_all_subscribers_prop(prop_input_counter);
		}
		if ((input_1 == 0) || (input_2 == 0) || (input_3 == 0) || (input_4 == 0)){
			if (on_off_state == false){
				on_off_state = true;
				inform_all_subscribers_prop(prop_in_on_off);
			}
		}
		else{
			if (on_off_state == true){
				on_off_state = false;
				inform_all_subscribers_prop(prop_in_on_off);
			}
		}
	}
}


/*******************************************************************
 *
 * initialize button's GPIO
 *
 * ******************************************************************/
void init_inputs_io(void){
	gpio_config_t io_conf;

	//interrupt on both edges
	io_conf.intr_type = GPIO_INTR_ANYEDGE;
	//bit mask of the pins
	io_conf.pin_bit_mask = GPIO_INPUT_MASK;
	//set as input mode
	io_conf.mode = GPIO_MODE_INPUT;
	//enable pull-up mode
	io_conf.pull_up_en = 1;
	io_conf.pull_down_en = 0;
	gpio_config(&io_conf);

	gpio_isr_handler_add(GPIO_INPUT_1, input_1_isr_handler, NULL);
	gpio_isr_handler_add(GPIO_INPUT_2, input_2_isr_handler, NULL);
	gpio_isr_handler_add(GPIO_INPUT_3, input_3_isr_handler, NULL);
	gpio_isr_handler_add(GPIO_INPUT_4, input_4_isr_handler, NULL);
}


/*****************************************************************
 *
 * Initialize button thing and all it's properties and event
 *
 * ****************************************************************/
thing_t *init_4_inputs(void){
	in_1 = false;
	in_2 = false;
	in_3 = false;
	in_4 = false;

	vSemaphoreCreateBinary(input_sem);
	xSemaphoreTake(input_sem, 0);
	init_inputs_io();
	
	inputs_mux = xSemaphoreCreateMutex();
	//create button thing
	iot_4_inputs = thing_init();
	
	iot_4_inputs -> id = "4-inputs";
	iot_4_inputs -> at_context = things_context;
	iot_4_inputs -> model_len = 2000;
	//set @type
	iot_4_inputs_type.at_type = "MultiLevelSwitch";
	iot_4_inputs_type.next = NULL;
	set_thing_type(iot_4_inputs, &iot_4_inputs_type);
	iot_4_inputs -> description = "4 inputs";
	
	//create input-1 property
	prop_input_1 = property_init(NULL, NULL);
	prop_input_1 -> id = "in-1";
	prop_input_1 -> description = "input-1 state";
	input_1_prop_type.at_type = "OpenProperty";
	input_1_prop_type.next = NULL;
	prop_input_1 -> at_type = &input_1_prop_type;
	prop_input_1 -> type = VAL_BOOLEAN;
	prop_input_1 -> value = &in_1;
	prop_input_1 -> title = "Input-1";
	prop_input_1 -> read_only = true;
	prop_input_1 -> set = NULL;
	prop_input_1 -> mux = inputs_mux;
	in_1 = gpio_get_level(GPIO_INPUT_1);

	add_property(iot_4_inputs, prop_input_1); //add property to thing
	
	//create input-2 property
	prop_input_2 = property_init(NULL, NULL);
	prop_input_2 -> id = "in-2";
	prop_input_2 -> description = "input-2 state";
	prop_input_2 -> at_type = &input_1_prop_type;
	prop_input_2 -> type = VAL_BOOLEAN;
	prop_input_2 -> value = &in_2;
	prop_input_2 -> title = "Input-2";
	prop_input_2 -> read_only = true;
	prop_input_2 -> set = NULL;
	prop_input_2 -> mux = inputs_mux;
	in_2 = gpio_get_level(GPIO_INPUT_2);

	add_property(iot_4_inputs, prop_input_2); //add property to thing
	
	//create input-3 property
	prop_input_3 = property_init(NULL, NULL);
	prop_input_3 -> id = "in-3";
	prop_input_3 -> description = "input-3 state";
	prop_input_3 -> at_type = &input_1_prop_type;
	prop_input_3 -> type = VAL_BOOLEAN;
	prop_input_3 -> value = &in_3;
	prop_input_3 -> title = "Input-3";
	prop_input_3 -> read_only = true;
	prop_input_3 -> set = NULL;
	prop_input_3 -> mux = inputs_mux;
	in_3 = gpio_get_level(GPIO_INPUT_3);

	add_property(iot_4_inputs, prop_input_3); //add property to thing
	
	//create input-4 property
	prop_input_4 = property_init(NULL, NULL);
	prop_input_4 -> id = "in-4";
	prop_input_4 -> description = "input-4 state";
	prop_input_4 -> at_type = &input_1_prop_type;
	prop_input_4 -> type = VAL_BOOLEAN;
	prop_input_4 -> value = &in_4;
	prop_input_4 -> title = "Input-4";
	prop_input_4 -> read_only = true;
	prop_input_4 -> set = NULL;
	prop_input_4 -> mux = inputs_mux;
	in_4 = gpio_get_level(GPIO_INPUT_4);

	add_property(iot_4_inputs, prop_input_4); //add property to thing
	
	//create on_off_state property
	prop_in_on_off = property_init(NULL, NULL);
	prop_in_on_off -> id = "on-off";
	prop_in_on_off -> description = "inputs state";
	on_off_prop_type.at_type = "OnOffProperty";
	on_off_prop_type.next = NULL;
	prop_in_on_off -> at_type = &on_off_prop_type;
	prop_in_on_off -> type = VAL_BOOLEAN;
	prop_in_on_off -> value = &on_off_state;
	prop_in_on_off -> title = "ON-OFF";
	prop_in_on_off -> read_only = true;
	prop_in_on_off -> set = NULL;
	prop_in_on_off -> mux = inputs_mux;
	if ((in_1 == 0) || (in_2 == 0) || (in_3 == 0) || (in_4 == 0)){
		on_off_state = true;
	}

	add_property(iot_4_inputs, prop_in_on_off); //add property to thing
	
	//create push counter property
	prop_input_counter = property_init(NULL, NULL);
	prop_input_counter -> id = "counter";
	prop_input_counter -> description = "input counter";
	input_counter_prop_type.at_type = "LevelProperty";
	input_counter_prop_type.next = NULL;
	prop_input_counter -> at_type = &input_counter_prop_type;
	prop_input_counter -> type = VAL_INTEGER;
	prop_input_counter -> value = &input_counter;
	prop_input_counter -> max_value.int_val = CNT_MAX;
	prop_input_counter -> min_value.int_val = 0;
	prop_input_counter -> title = "Counter";
	prop_input_counter -> read_only = true;
	prop_input_counter -> set = NULL;
	prop_input_counter -> mux = inputs_mux;

	add_property(iot_4_inputs, prop_input_counter); //add property to thing
	
	if (input_sem != NULL){
		xTaskCreate(&inputs_fun, "inputs_task",
					configMINIMAL_STACK_SIZE * 4, NULL, 0, NULL);
	}

	return iot_4_inputs;
}
