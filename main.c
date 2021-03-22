#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

static volatile int run = 1;

void exitWithError(const char* err) {
	perror(err);
	exit(1);
}

void writeToFile(int file, const char* message, const char* err) {
	if (write(file, message, strlen(message)) != strlen(message))
		exitWithError(err);
}

void writeFileWithName(int pin, const char* value, const char* name) {

	char* path;
	char* err;
	char* openErr;

	asprintf(&path, "/sys/class/gpio/gpio%d/%s", pin, name);
	asprintf(&err, "Unable to write file /sys/class/gpio/gpio%d/%s", pin, name);
	asprintf(&openErr, "Uable to open file %s", path);

	int file = open(path, O_WRONLY);
	if (file == -1) exitWithError(openErr);

	writeToFile(file, value, err);

	close(file);

	free(path);
	free(err);
	free(openErr);
}

void signalHandler(int dumy) {
	run = 0;
}

int main() {

	signal(SIGINT, signalHandler);

	int pins[] = { 26, 19 };
	int numOfPins = 2;

	// init pin
	//
	int file = open("/sys/class/gpio/export", O_WRONLY);
	if (file == -1) exitWithError("Unable to open /sys/class/gpio/export");

	for (int i = 0; i < numOfPins; ++i) {
		char* pinStr;
		asprintf(&pinStr, "%d", pins[i]);
		writeToFile(file, pinStr, "Unable to write file /sys/class/gpio/export");
		free(pinStr);
	}

	close(file);

	// set pin direction
	//
	for (int i = 0; i < numOfPins; ++i)
		writeFileWithName(pins[i], "out", "direction");

	/*
	file = open("/sys/class/gpio/gpio26/direction", O_WRONLY);
	if (file == -1) exitWithError("Unable to open /sys/class/gpio/gpio26/direction");

	writeToFile(file, "out", "Unable to write file /sys/class/gpio/gpio26/direction");

	close(file);

	file = open("/sys/class/gpio/gpio19/direction", O_WRONLY);
	if (file == -1) exitWithError("Unable to open /sys/class/gpio/gpio19/direction");

	writeToFile(file, "out", "Unable to write file /sys/class/gpio/gpio19/direction");

	close(file);*/

	// write value to file
	//

	int power = 1;
	int sleep = 250000;
	const char* filename = "value";

	while (run) {
		for (int i = 0; i < numOfPins; ++i) {
			writeFileWithName(pins[i], power == 1 ? "1" : "0", filename);
			usleep(sleep);
		}
		power = 1 - power;
	}


	close(file);

	// unxport pin
	//
	file = open("/sys/class/gpio/unexport", O_WRONLY);
	if (file == -1) exitWithError("Unable to open /sys/class/gpio/unexport");

	for (int i = 0; i < numOfPins; ++i) {
		char* pinStr;
		asprintf(&pinStr, "%d", pins[i]);
		writeToFile(file, pinStr, "Unable to write file /sys/class/gpio/unexport");
		free(pinStr);
	}

	close(file);

	return 0;
}
