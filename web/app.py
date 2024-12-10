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


def read_thresholds():
    data = read_file("/tmp/thresholds", "18.0,25.0,16.0,18.0,20.0")
    values = data.split(",")
    if len(values) == 5:
        return {
            "temp_day_lower": float(values[0]),
            "temp_day_upper": float(values[1]),
            "temp_night_lower": float(values[2]),
            "temp_night_upper": float(values[3]),
            "soil_dry": float(values[4]),
        }
    return None


def get_sensor_status(value, lower, upper, type_name):
    if value <= lower:
        return f"{type_name}Low!"
    elif value >= upper:
        return f"{type_name}High!"
    return ""


def check_status(temperature, soil_moisture, brightness, thresholds, is_daytime):
    problems = []

    # 온도 체크
    temp_lower = (
        thresholds["temp_day_lower"] if is_daytime else thresholds["temp_night_lower"]
    )
    temp_upper = (
        thresholds["temp_day_upper"] if is_daytime else thresholds["temp_night_upper"]
    )

    if temperature < temp_lower:
        problems.append("Cold!")
    elif temperature > temp_upper:
        problems.append("Hot!")

    # 토양 습도 체크
    if soil_moisture <= thresholds["soil_dry"]:
        problems.append("Dry!")

    # 밝기 체크 (낮 시간대일 때만)
    if is_daytime and brightness == "dark":
        problems.append("Dark!")

    return " ".join(problems) if problems else "정상"


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/data")
def get_data():
    # 센서값 읽기
    temperature = float(read_file("/tmp/temperature", "20.0"))
    brightness = read_file("/tmp/brightness", "dark")
    soil_moisture = float(read_file("/tmp/soilmoisture", "0.0"))
    thresholds = read_thresholds()

    # 현재 시간 확인
    current_hour = time.localtime().tm_hour
    is_daytime = 7 <= current_hour < 19

    # 상태 판단
    status = check_status(
        temperature, soil_moisture, brightness, thresholds, is_daytime
    )

    return jsonify(
        {
            "temperature": f"{temperature:.1f}°C",
            "brightness": brightness,
            "soil_moisture": f"{soil_moisture:.1f}%",
            "status": status,
            "thresholds": thresholds,
            "is_daytime": is_daytime,
            "raw_values": {  # 프론트엔드에서 필요할 수 있는 원시 데이터
                "temperature": temperature,
                "soil_moisture": soil_moisture,
                "current_hour": current_hour,
            },
        }
    )


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)
