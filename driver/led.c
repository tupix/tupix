#define GPIO_BASE (0x7E200000 - 0x3F000000)
#define YELLOW_LED 7
#define GPF_BITS 3

enum gpio_func {
	gpio_input = 0x0, 	// GPIO Pin is an input
	gpio_output = 0x1, 	// GPIO Pin is an output
};

struct gpio {
    unsigned int func[6];
    unsigned int unused0;
    unsigned int set[2];
    unsigned int unused1;
    unsigned int clr[2];
};

static volatile
struct gpio * const gpio_port = (struct gpio *)GPIO_BASE;

void yellow_on(void)
{
    /* Initialisieren */
    gpio_port->func[0] = gpio_output << (YELLOW_LED * GPF_BITS);

    /* Anschalten */
    gpio_port->set[0] = 1 << YELLOW_LED;
}
