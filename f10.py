from flask import Flask, request
import requests
import RPi.GPIO as GPIO
import time

app = Flask(__name__)

MAKE_WEBHOOK_URL = 'https://hook.eu2.make.com/sge5luoqxnmc6drqhjd54xsjaen86'

# Setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(17, GPIO.OUT)

@app.route('/data', methods=['POST'])
def receive_data():
    temperature = request.form.get('temperature')
    humidity = request.form.get('humidity')
    if temperature and humidity:
        print(f"Received data - Temperature: {temperature}°C, Humidity: {humidity}%")

        # Blink pin 17
        GPIO.output(17, GPIO.HIGH)
        time.sleep(1)
        GPIO.output(17, GPIO.LOW)

        # Send data to Make webhook
        payload = {
            'temperature': temperature,
            'humidity': humidity
        }
        response = requests.post(MAKE_WEBHOOK_URL, json=payload)

        if response.status_code == 200:
            return "Data received and sent to Make successfully", 200
        else:
            return f"Data received but failed to send to Make. Status code: {response.status_code}", 500
    else:
        return "Invalid data", 400

if __name__ == '__main__':
    try:
        app.run(host='0.0.0.0', port=5000)
    except KeyboardInterrupt:
        pass
    finally:
        GPIO.cleanup()

