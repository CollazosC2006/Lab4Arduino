#include <ETH.h> 
#define ETH_ADDR        1
#define ETH_POWER_PIN   16
#define ETH_POWER_PIN_ALTERNATIVE 16 
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
#define ETH_TYPE       ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN 

void setup() {
Serial.begin(115200);
ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE);

//ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE); 
while(!((uint32_t)ETH.localIP())){};
}
void loop() {
if (ETH.linkUp()) {
Serial.print("Conectado, IP: ");
Serial.println(ETH.localIP());
} else {
Serial.println("Desconectado");
}
delay(3000);
}        