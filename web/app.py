# app.py
from flask import Flask, render_template, jsonify
import threading
import time

app = Flask(__name__)


def read_file(path, default="0"):
    try:
        with open(path, "r") as f:
            return f.read().strip()
    except:
        return default


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/data")
def get_data():
    temperature = float(read_file("/tmp/temperature", "20.0"))
    brightness = read_file("/tmp/brightness", "dark")
    soil_moisture = float(read_file("/tmp/soilmoisture", "0.0"))
    status = read_file("/tmp/status", "정상")

    # 상태값이 비어있으면 '정상'으로 표시
    if not status.strip():
        status = "정상"

    return jsonify(
        {
            "temperature": f"{temperature:.1f}°C",
            "brightness": brightness,
            "soil_moisture": f"{soil_moisture:.1f}%",
            "status": status,
        }
    )


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)
