#include <Arduino.h>

// Pin assignment (CH32V003 SOP8 package)
#define RADAR_SDA    PC1  // Physical pin 5
#define RADAR_SCL    PC2  // Physical pin 6
#define RADAR_IIC_EN PC4  // Physical pin 7


// QUICK SETTINGS 

//#######################################################################

// [1] Sensitivity & Distance
// #define RADAR_THRESHOLD 0x0180  // ~10-12 meters 
// #define RADAR_THRESHOLD 0x0340  // ~8-10 meters
// #define RADAR_THRESHOLD 0x0500  // ~6-8 meters
#define RADAR_THRESHOLD    0x0A00  // ~4-6 meters 
// #define RADAR_THRESHOLD 0x1400  // ~2-4 meters
// #define RADAR_THRESHOLD 0x2800  // ~1-2 meters
// #define RADAR_THRESHOLD 0x5000  // ~0.5-1 meter 


//#######################################################################


// [2] OUT pin hold delay after motion stops (in milliseconds)
// #define RADAR_DELAY_MS  100     // 0.1 sec 
#define RADAR_DELAY_MS     500     // 0.5 sec 
// #define RADAR_DELAY_MS  2000    // 2 sec
// #define RADAR_DELAY_MS  5000    // 5 sec
// #define RADAR_DELAY_MS  10000   // 10 sec
// #define RADAR_DELAY_MS  30000   // 30 sec 
// #define RADAR_DELAY_MS  60000   // 1 min


//#######################################################################


// [3] Lockout / blind time after OUT drops LOW (in milliseconds)
#define RADAR_LOCKOUT_MS   0       // 0 ms (Instant re-triggering)
// #define RADAR_LOCKOUT_MS 1000   // 1 sec blind time
// #define RADAR_LOCKOUT_MS 3000   // 3 sec blind time


//#######################################################################


// [4] RF Transmitter Power
#define RADAR_TX_POWER     HLKRadar::TPOWER_MAX // 100% power (Maximum distance)
// #define RADAR_TX_POWER  HLKRadar::TPOWER_2   // ~70% power
// #define RADAR_TX_POWER  HLKRadar::TPOWER_4   // ~40% power
// #define RADAR_TX_POWER  HLKRadar::TPOWER_MIN // Minimum power (for tight enclosures)


//#######################################################################


// [5] Operating & Power Mode of RF Block
#define RADAR_WORK_MODE    HLKRadar::PSM_PULSE       // Pulsed (Power saving)
// #define RADAR_WORK_MODE HLKRadar::PSM_CONTINUOUS // Continuous (Maximum response speed)


//#######################################################################


// [6] ADC Sampling Frequency
// #define RADAR_ADC_FREQ  HLKRadar::ADC_SF_1KHz  
#define RADAR_ADC_FREQ     HLKRadar::ADC_SF_2KHz   
// #define RADAR_ADC_FREQ  HLKRadar::ADC_SF_4KHz  
// #define RADAR_ADC_FREQ  HLKRadar::ADC_SF_16KHz 


//#######################################################################

// [7] Adaptive Noise Threshold
#define RADAR_NOISE_UPDATE 0x0155  // Factory default balanced value
// #define RADAR_NOISE_UPDATE 0x01AA  // Increased noise immunity (for rooms with fans)


//#######################################################################


// ============================================================================
// DRIVER INTERNAL IMPLEMENTATION
// ============================================================================

class HLKRadar {
public:
    enum PsmMode {
        PSM_PULSE      = 0x20,
        PSM_CONTINUOUS = 0xA0
    };

    enum AdcFs {
        ADC_SF_1KHz  = 0,
        ADC_SF_2KHz  = 1,
        ADC_SF_4KHz  = 2,
        ADC_SF_16KHz = 3
    };

    enum TxPower {
        TPOWER_MAX = 0,
        TPOWER_1   = 1,
        TPOWER_2   = 2,
        TPOWER_3   = 3,
        TPOWER_4   = 4,
        TPOWER_5   = 5,
        TPOWER_6   = 6,
        TPOWER_MIN = 7
    };

    HLKRadar(uint8_t enPin, uint8_t sdaPin, uint8_t sclPin)
        : _en(enPin), _sda(sdaPin), _scl(sclPin) {}

    bool begin() {
        pinMode(_en, OUTPUT);
        digitalWrite(_en, HIGH); 
        delay(200); 

        i2c_init();
        i2c_start();
        bool status = i2c_write_byte(_addr << 1);
        i2c_stop();
        return status;
    }

    void loadDefaultConfig() {
        digitalWrite(_en, HIGH); 
        delay(10);   
        
        for (size_t i = 0; i < 5; i++) {
            if (writeReg(0x13, 0x9b)) delayMicroseconds(10); 
            readReg(0x13); 
            delayMicroseconds(10); 
        }
        
        writeReg(0x24, 0x03); 
        writeReg(0x04, 0x20); 
        writeReg(0x10, 0x20); 
        writeReg(0x03, 0x45); 
        writeReg(0x1C, 0x21); 

        writeReg(0x18, 0x5a); 
        writeReg(0x19, 0x01); 

        writeReg(0x1A, 0x55); 
        writeReg(0x1B, 0x01); 

        writeReg(0x1D, 0x80); 
        writeReg(0x1E, 0x0C); 
        writeReg(0x1F, 0x00); 

        writeReg(0x20, 0x00); 
        writeReg(0x21, 0x7D); 
        writeReg(0x22, 0x00); 

        writeReg(0x23, 0x0C); 

        delay(1000); 
    }

    void setIoValOutput(uint8_t status) {
        writeReg(0X24, status ? 0X07 : 0X03); 
    }

    void setWayOfWorking(PsmMode mode) {
        writeReg(0X04, mode); 
    }

    void setADCSamplingFrequency(AdcFs freq) {
        switch (freq) {
            case ADC_SF_1KHz:  writeReg(0X10, 0X20); break; 
            case ADC_SF_2KHz:  writeReg(0X10, 0X10); break; 
            case ADC_SF_4KHz:  writeReg(0X10, 0X08); break; 
            case ADC_SF_16KHz: writeReg(0X10, 0X02); break; 
        }
    }

    void setTransmittingPower(TxPower power) {
        writeReg(0X03, 0X40 + power); 
    }

    void setInductionThreshold(uint16_t threshold) {
        writeReg(0X18, (threshold & 0XFF));        
        writeReg(0X19, ((threshold >> 8) & 0XFF)); 
    }

    void setNoiseUpdate(uint16_t noise) {
        writeReg(0X1A, (noise & 0XFF));        
        writeReg(0X1B, ((noise >> 8) & 0XFF)); 
    }

    void setInductionDelayTime(uint32_t delayMs) {
        uint32_t timer_hex = delayMs * 32; 
        writeReg(0X1D, (timer_hex & 0XFF));         
        writeReg(0X1E, ((timer_hex >> 8) & 0XFF));  
        writeReg(0X1F, ((timer_hex >> 16) & 0XFF)); 
    }

    void setBlockadeTime(uint32_t blockadeMs) {
        uint32_t timer_hex = blockadeMs * 32; 
        writeReg(0X20, (timer_hex & 0XFF));         
        writeReg(0X21, ((timer_hex >> 8) & 0XFF));  
        writeReg(0X22, ((timer_hex >> 16) & 0XFF)); 
    }

private:
    uint8_t _en, _sda, _scl;
    const uint8_t _addr = 0x71; 

    void i2c_init() {
        sda_high();
        scl_high();
        delayMicroseconds(5);
    }

    void i2c_start() {
        sda_high(); scl_high(); delayMicroseconds(5);
        sda_low();  delayMicroseconds(5);
        scl_low();  delayMicroseconds(5);
    }

    void i2c_stop() {
        sda_low();  scl_low();  delayMicroseconds(5);
        scl_high(); delayMicroseconds(5);
        sda_high(); delayMicroseconds(5);
    }

    bool i2c_write_byte(uint8_t byte) {
        for (uint8_t i = 0; i < 8; i++) {
            if (byte & 0x80) sda_high();
            else sda_low();
            byte <<= 1;
            delayMicroseconds(4);
            scl_high(); delayMicroseconds(5);
            scl_low();  delayMicroseconds(1);
        }
        
        sda_high();
        delayMicroseconds(4);
        scl_high();
        delayMicroseconds(4);
        bool ack = (sda_read() == false);
        scl_low();
        delayMicroseconds(2);
        return ack;
    }

    uint8_t i2c_read_byte(bool ack) {
        uint8_t byte = 0;
        sda_high();
        for (uint8_t i = 0; i < 8; i++) {
            byte <<= 1;
            scl_high(); delayMicroseconds(4);
            if (sda_read()) byte |= 0x01;
            scl_low();  delayMicroseconds(4);
        }
        
        if (ack) sda_low();
        else sda_high();
        delayMicroseconds(2);
        scl_high(); delayMicroseconds(5);
        scl_low();  sda_high();
        delayMicroseconds(2);
        return byte;
    }

    void sda_high() { pinMode(_sda, INPUT); }
    void sda_low()  { pinMode(_sda, OUTPUT); digitalWrite(_sda, LOW); }
    void scl_high() { pinMode(_scl, INPUT); }
    void scl_low()  { pinMode(_scl, OUTPUT); digitalWrite(_scl, LOW); }
    bool sda_read() { return digitalRead(_sda) == HIGH; }

    bool writeReg(uint8_t reg, uint8_t value) {
        i2c_start();
        if (!i2c_write_byte(_addr << 1)) { i2c_stop(); return false; }
        if (!i2c_write_byte(reg))       { i2c_stop(); return false; }
        if (!i2c_write_byte(value))     { i2c_stop(); return false; }
        i2c_stop();
        return true;
    }

    uint8_t readReg(uint8_t reg) {
        i2c_start();
        if (!i2c_write_byte(_addr << 1)) { i2c_stop(); return 0; }
        if (!i2c_write_byte(reg))       { i2c_stop(); return 0; }
        
        i2c_start(); 
        if (!i2c_write_byte((_addr << 1) | 1)) { i2c_stop(); return 0; }
        uint8_t val = i2c_read_byte(false); 
        i2c_stop();
        return val;
    }
};

HLKRadar radar(RADAR_IIC_EN, RADAR_SDA, RADAR_SCL);


void setup() {
    delay(7000); // The radar takes time to initialize at boot, so wait a bit.
    
    while (!radar.begin()) {
        delay(500); 
    }
    
    radar.loadDefaultConfig(); 

    // Apply selected configuration values from above
    radar.setInductionThreshold(RADAR_THRESHOLD);
    radar.setInductionDelayTime(RADAR_DELAY_MS);
    radar.setBlockadeTime(RADAR_LOCKOUT_MS);
    radar.setTransmittingPower(RADAR_TX_POWER);
    radar.setWayOfWorking(RADAR_WORK_MODE);
    radar.setADCSamplingFrequency(RADAR_ADC_FREQ);
    radar.setNoiseUpdate(RADAR_NOISE_UPDATE);

    // Set SDA and SCL to High-Z mode
    pinMode(RADAR_SDA, INPUT);
    pinMode(RADAR_SCL, INPUT);

    // CRITICAL: Keep EN pin HIGH (otherwise the radar resets its registers!)
    pinMode(RADAR_IIC_EN, OUTPUT);
    digitalWrite(RADAR_IIC_EN, HIGH);

    // Stop the CH32V003 core (once registers are saved, the radar retains them until power-loss)
    while (1) {
        __asm__ volatile ("wfi");
    }
}

void loop() {

}
