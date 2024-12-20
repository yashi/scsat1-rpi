/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "temp.h"

#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <systemd/sd-journal.h>
#include "cspd.h"

#define I2C_DEV_NO            (1U)
#define I2C_TMP175_SLAVE_ADDR (0x4E)
#define I2C_TMP175_TEMP_REG   (0x00)

static int get_tmp175_temp(uint16_t *temp)
{
	int fd;
	unsigned char reg = I2C_TMP175_TEMP_REG;
	unsigned char dat[2];
	char i2c_dev_fn[64];

	sprintf(i2c_dev_fn, "/dev/i2c-%d", I2C_DEV_NO);
	if ((fd = open(i2c_dev_fn, O_RDWR)) < 0) {
		sd_journal_print(LOG_ERR, "Faild to open i2c port\n");
		return -1;
	}

	if (ioctl(fd, I2C_SLAVE, I2C_TMP175_SLAVE_ADDR) < 0) {
		sd_journal_print(LOG_ERR, "Unable to get bus access to talk to slave\n");
		return -1;
	}

	if ((write(fd, &reg, 1)) != 1) {
		sd_journal_print(LOG_ERR, "Error writing to i2c slave\n");
		return -1;
	}

	if (read(fd, dat, 2) != 2) {
		sd_journal_print(LOG_ERR, "Error reading from i2c slave\n");
		return -1;
	}

	dat[1] = dat[1] >> 4;
	*temp = (dat[0] << 8) | dat[1];

	close(fd);

	return 0;
}

void get_temp_service(uint8_t command_id, csp_packet_t *packet)
{
	uint16_t temp = 0;
	struct hwtest_temp_telemetry tlm;

	(void)get_tmp175_temp(&temp);

	tlm.telemetry_id = command_id;
	tlm.temp_raw = temp;
	memcpy(packet->data, &tlm, sizeof(tlm));
	packet->length = sizeof(tlm);
	csp_sendto_reply(packet, packet, CSP_O_SAME);
}
