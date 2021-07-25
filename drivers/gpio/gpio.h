#ifndef __GPIO_H__
#define __GPIO_H__


struct gpio_t{
	char * name;
	int base;
	int ngpio;
};

int gpio_is_valid(int number);

int gpio_direction_input(unsigned gpio);
int gpio_direction_output(unsigned gpio, int value);


int gpio_get_value(unsigned gpio);
void gpio_set_value(unsigned gpio, int value);


int gpio_request(unsigned gpio, const char *label);
void gpio_free(unsigned gpio);

#endif