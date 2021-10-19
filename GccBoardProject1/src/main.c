
#include <asf.h>
#define F_CPU 8000000UL

#define CYCLES_IN_DLYTICKS_FUNC        8
#define MS_TO_DLYTICKS(ms)          (U32)(F_CPU / 1000 * ms / CYCLES_IN_DLYTICKS_FUNC) // ((float)(F_CPU)) / 1000.0
#define DelayTicks(ticks)            {volatile U32 n=ticks; while(n--);}//takes 8 cycles
#define DelayMs(ms)                    DelayTicks(MS_TO_DLYTICKS(ms))//uses 20bytes


volatile uint8_t pulse = 0;  
int buff1 [30];
int buff2 [50];

float flowRate = 0.00;
float kFactor = 98.00;                   // conversion factor from frequency to flow rate in LPM.
 int ndigit ;

void configure_extint_channel(void);
void configure_extint_callbacks(void);
void extint_detection_callback(void);

void configure_extint_channel(void)
{

	struct extint_chan_conf config_extint_chan;
	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin           = PIN_PA07A_EIC_EXTINT7;
	config_extint_chan.gpio_pin_mux       = MUX_PA07A_EIC_EXTINT7;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_RISING;
	extint_chan_set_config(7, &config_extint_chan);


}

void configure_extint_callbacks(void)
{

extint_register_callback(extint_detection_callback,7,EXTINT_CALLBACK_TYPE_DETECT);
extint_chan_enable_callback(7,EXTINT_CALLBACK_TYPE_DETECT);
}

void extint_detection_callback(void)
{
	bool pin_state = port_pin_get_input_level(PIN_PA07);
    port_pin_set_output_level(LED_0_PIN, pin_state);
	
	pulse++;
}


void configure_usart(void);
struct usart_module usart_instance;


void configure_usart(void)
{
	
	struct usart_config config_usart;

	usart_get_config_defaults(&config_usart);
	


		config_usart.baudrate    = 115200;
		config_usart.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
		config_usart.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
		config_usart.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
		config_usart.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
		config_usart.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;

		while (usart_init(&usart_instance,
		EDBG_CDC_MODULE, &config_usart) != STATUS_OK) {
		}

	usart_enable(&usart_instance);
	
}

int main(void)
{
	system_init();
	configure_usart();
	
    configure_extint_channel();
	configure_extint_callbacks();
	system_interrupt_enable_global();
	

	uint16_t temp;

	while (true) {
		pulse = 0;
		system_interrupt_enable_global();
		DelayMs(1000);
		system_interrupt_disable_global();
		
		flowRate = 33.83 * pulse/kFactor;
		
		itoa(pulse, buff1, 10);
		
		char *gcvtf(float flowRate, int ndigit, char *buff2);		
		      gcvtf(flowRate,2,buff2);
		
		usart_write_buffer_wait(&usart_instance, buff1, sizeof(buff1));
		uint8_t string1[] = "  pulse \t";
		usart_write_buffer_wait(&usart_instance, string1, sizeof(string1));
		
		usart_write_buffer_wait(&usart_instance, buff2, sizeof(buff2));
		uint8_t string2[] = "  Oz/min \r\n";
		usart_write_buffer_wait(&usart_instance, string2, sizeof(string2));
		
		
		if (usart_read_wait(&usart_instance, &temp) == STATUS_OK) {
			while (usart_write_wait(&usart_instance, temp) != STATUS_OK) {
			}
			
		}
	}

}