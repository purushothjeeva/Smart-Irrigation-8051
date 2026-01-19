#include <reg51.h>

// LCD Pins
sbit RS = P2^0;
sbit RW = P2^1;
sbit EN = P2^2;
#define LCD P0

// ADC0804 Pins
sbit ADC_RD   = P3^0;
sbit ADC_WR   = P3^1;
sbit ADC_INTR = P3^2;

// Extra Smart Features
sbit LDR    = P3^3;   // Day/Night detection
sbit BUZZER = P3^4;   // Fault alert
sbit RELAY  = P2^7;   // Pump control

unsigned char prev_moisture = 0;
unsigned char threshold = 80;

// Delay
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 1275; j++);
}

// LCD Functions
void lcd_cmd(unsigned char cmd) {
    LCD = cmd;
    RS = 0; RW = 0; EN = 1;
    delay_ms(2);
    EN = 0;
}

void lcd_data(unsigned char dat) {
    LCD = dat;
    RS = 1; RW = 0; EN = 1;
    delay_ms(2);
    EN = 0;
}

void lcd_init() {
    lcd_cmd(0x38);
    lcd_cmd(0x0C);
    lcd_cmd(0x01);
    lcd_cmd(0x80);
}

void lcd_string(char *s) {
    while(*s) lcd_data(*s++);
}

// ADC Read
unsigned char read_adc() {
    unsigned char val;
    ADC_WR = 0;
    delay_ms(2);
    ADC_WR = 1;

    while(ADC_INTR == 1);
    ADC_RD = 0;
    val = P1;      // ADC data on Port1
    ADC_RD = 1;
    return val;
}

// Smart Control Logic
void smart_control(unsigned char moisture) {

    // Self-learning threshold
    threshold = (threshold + moisture) / 2;

    // Day/Night logic
    if(LDR == 1) {       // Daytime
        RELAY = 0;
        lcd_cmd(0xC0);
        lcd_string("Day Mode     ");
        return;
    }

    // Pump control
    if(moisture < threshold) {
        RELAY = 1;
        lcd_cmd(0xC0);
        lcd_string("Pump: ON     ");
    } else {
        RELAY = 0;
        lcd_cmd(0xC0);
        lcd_string("Pump: OFF    ");
    }

    // Dry-run protection
    if(RELAY == 1 && (moisture <= prev_moisture)) {
        RELAY = 0;
        BUZZER = 1;
        lcd_cmd(0xC0);
        lcd_string("Pump Error!  ");
        delay_ms(2000);
        BUZZER = 0;
    }

    prev_moisture = moisture;
}

void main() {
    unsigned char moisture;

    lcd_init();
    lcd_string("Smart Irrigate");

    while(1) {
        moisture = read_adc();
        smart_control(moisture);
        delay_ms(1000);
    }
}
