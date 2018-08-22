from argparse import ArgumentParser
import pyowm
import json


def get_api_key():
    try:
        with open("openweather_api_key.txt", "r") as api_file:
            line = api_file.readline()
            if line is None or line is "":
                raise IOError('File is blank or invalid.')
            return line
    except IOError as error:
        print(repr(error), "Please create a file in the same location as this script containing your OpenWeatherMap API"
                           " key. If you do not have a key you can get one for free here https://openweathermap.org/. "
                           "The file should contain only your API key on line 0 and be called "
                           "'openweather_api_key.txt'.")
        return None


def get_value(obj, key):
    try:
        if type(obj) is not dict or obj is "":
            raise KeyError()
        value = obj[key]
        return value if value is not None else ""
    except KeyError:
        return ""


def get_weather_data(observation, verbose=False):
    json_resp = observation.to_JSON()
    json_acceptable_string = json_resp.replace("'", "\"")
    json_weather_data = json.loads(json_acceptable_string)

    weather = json_weather_data['Weather']
    location = json_weather_data['Location']
    reception_time = json_weather_data['reception_time']

    weather_info = {
        "Weather status": get_value(weather, "status"),
        "Detailed weather status": get_value(weather, "detailed_status"),
        "Visibility distance": get_value(weather, "visibility_distance"),
        "Pressure": get_value(get_value(weather, "pressure"), "press"),
        "Pressure sea level": get_value(get_value(weather, "pressure"), "sea_level"),
        "Weather code": get_value(weather, "weather_code"),
        "Sunrise time": get_value(weather, "sunrise_time"),
        "Sunset time": get_value(weather, "sunset_time"),
        "Reference time": get_value(weather, "reference_time"),
        "Humidity": get_value(weather, "humidity"),
        "Wind speed": get_value(get_value(weather, "wind"), "speed"),
        "Wind degree": get_value(get_value(weather, "wind"), "deg"),
        "Rain in the last 3 hours": get_value(get_value(weather, "rain"), "3h"),
        "Temperature": get_value(get_value(weather, "temperature"), "temp"),
        "Clouds": get_value(weather, "clouds"),
        "Weather info reception time": reception_time,
        'Weather location ID': get_value(location, 'ID'),
        'Weather country code': get_value(location, 'country'),
        'Weather location lon': get_value(get_value(location, 'coordinates'), 'lat'),
        'Weather location lat': get_value(get_value(location, 'coordinates'), 'lon'),
        'Weather location name': get_value(location, 'name')
    }

    if verbose:
        for k, v in weather_info.items():
            print(k, v)

    return weather_info


def non_nested_dict_to_csv(data):
    for k in sorted(data.keys()):
        print(k, ",", data[k], sep="")


def get_weather(latitude, longitude):
    api_key = get_api_key()

    if api_key is None:
        print("Invalid API key.")
        return

    owm = pyowm.OWM(api_key)
    observation = owm.weather_at_coords(lat=latitude, lon=longitude)
    non_nested_dict_to_csv(get_weather_data(observation))


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("-lat", "--latitude", dest="lat", help="Latitude of data location", default=52.268491)
    parser.add_argument("-lon", "--longitude", dest="lon", help="Longitude of data location", default=-0.523832)
    args = parser.parse_args()
    lat = args.lat
    lon = args.lon
    get_weather(lat, lon)
