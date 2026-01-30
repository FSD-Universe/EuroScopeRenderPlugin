#  Copyright (c) 2026 Half_nothing
#  SPDX-License-Identifier: MIT

def read_data(data: list[str]):
    print("input data(q or empty line to exit):")
    while True:
        tmp = input()
        data.append(tmp)
        if tmp == '' or tmp == 'q':
            break


def transform(lat: str, lon: str) -> tuple[float, float]:
    north = 1 if lat[0] == 'N' else -1
    east = 1 if lon[0] == 'E' else -1
    lat = lat[1:].split('.')
    lon = lon[1:].split('.')
    lat = float(lat[0]) + float(lat[1]) / 60 + float(lat[2]) / 3600 + float(lat[3]) / 360000 * north
    lon = float(lon[0]) + float(lon[1]) / 60 + float(lon[2]) / 3600 + float(lon[3]) / 360000 * east
    return lat, lon


def main():
    data = []
    read_data(data)
    data = [i.split(':') for i in data]
    print("coordinates:")
    for i in data:
        if len(i) == 0 or i[0] != 'COORD':
            continue
        latitude, longitude = transform(i[1], i[2])
        print(f"  - [{longitude:.6f}, {latitude:.6f}]")


if __name__ == '__main__':
    main()
