# Транспортный справочник

## Описание
**Транспортный справочник** - это инструмент на базе C++, предназначенный для хранения данных об автобусах и остановках и их обработки. В зависимости от задачи, он выводит информацию об остановках и/или маршрутах, строит карты со всеми объектами и/или осуществляет расчет кратчайшего маршрута из точки А в точку Б. **Транспортный справочник** работает с такими форматами, как JSON и SVG, а также имеет возможность выгружать базу данных в бинарный файл и работать с ним. 

## Возможности
* Для наполнения справочника используется функция *Make_Base(...)*. Она обрабатывает полученные команды, заполняет справочник и выгружает его данные в бинарный файл. Сами команды передаются в JSON формате и выглядят следующим образом:
    1) *"serialization_settings"* - указание имени файла, в который будет выгружаться база данных. Пример:
        
        ```
        "serialization_settings": {
            "file": "transport_catalogue.db"
        }
        ```
    2) *"base_requests"* - передача данных об остановках и автобусах. В них передается тип объекта, имя и другая информация, характерная только для определенного типа. Так для остановки дополнительно передаются координаты широты и долготы, а также при необходимости указывается список соседних остановок с расстояними до них. В автобусе же указывается список остановок, через которые проходит маршрут и является ли маршрут кольцевым. Пример:
        
        ```
        "base_requests": [
            {
                "type": "Stop",
                "name": "The Riviera Bridge",
                "latitude": 43.587795,
                "longitude": 39.716901,
                "road_distances": {
                    "Marine Station": 850,
                    "Sochi Hotel": 1740
                }
            },
            {
                "type": "Bus",
                "name": "14",
                "stops": [
                    "Lisa Chaikina Street",
                    "Riviera Bridge",
                    "Sochi Hotel",
                    "Kubanskaya Street",
                    "On demand",
                    "Dokuchaeva Street",
                    "Lisa Chaikina Street"
                ],
                "is_roundtrip": true
            }
        ]
        ```
    3) *"render_settings"* - задание настроек рендера карты. Здесь указывается ширина изображения, его высота, отступ, радиус меток остановок на карте, толщина линий, размеры и смещение текста имен автобусов и остановок, цвет и ширина подложек, а также цветовая палитра, на основе которой и будет выбираться цвет для объектов на карте. Пример:
        
        ```
        "render_settings": {
           "width": 1200,
           "height": 500,
           "padding": 50,
           "stop_radius": 5,
           "line_width": 14,
           "bus_label_font_size": 20,
           "bus_label_offset": [
               7,
               15
           ],
           "stop_label_font_size": 18,
           "stop_label_offset": [
               7,
               -3
           ],
           "underlayer_color": [
               255,
               255,
               255,
               0.85
           ],
           "underlayer_width": 3,
           "color_palette": [
               "green",
               [
                   255,
                   160,
                   0
               ],
               "red"
           ]
        }
        ```
    4) *"routing_settings"* - задание настроек построения маршрута. Здесь передается среднее время ожидания на остановках и средняя скорость автобусов. Пример:
        
        ```
        "routing_settings": {
           "bus_wait_time": 2,
           "bus_velocity": 30
        }
        ```
* Для обработки запросов используется функция *ProcessRequests(...)*. Она заполняет **транспортный справочник** из бинарного фала и выдает результаты обработки, согласно полученным командам. Здесь команды схожим образом передаются в JSON формате и выглядят так:
    1) *"serialization_settings"* - указание имени файла, из которого будет загружаться база данных. Структура команды выглядит аналогично представленной выше в функции *Make_Base(...)*.
    2) *"stat_requests"* - сами запросы к **транспортному справочнику**. Они содержат id запроса, его тип и другую информацию, характерную только для определенного типа. Запрос информации по остановке и автобусу дополнительно содержит имя интересующего нас объекта, а маршрут указывает точки начала и конца пути. Пример:
        
        ```
        "stat_requests": [
            {
                "id": 218563507,
                "type": "Bus",
                "name": "14"
            },
            {
                "id": 508658276,
                "type": "Stop",
                "name": "The Riviera Bridge"
            },
            {
                "id": 1964680131,
                "type": "Route",
                "from": "Lisa Chaikina Street",
                "to": "Sochi Hotel"
            },
            {
                "id": 1359372752,
                "type": "Map"
            }
        ]
        ```
* Функция *RuntimeProcessRequests(...)* выполняет те же действия, что и две предыдущие, однако делает это непосредственно во время выполнения программы. По этой причине, данная функция не обрабатывает команду *"serialization_settings"*.

## Требования
Для установки **транспортного каталога** требуется:
1) Cистема сборки проектов CMake, версии не ниже 3.11;
2) IDE Visual Studio 2022;
3) Cтатические библиотеки инструмента сериализации Google Protobuf (релизная и отладочная версии). Версия Protobuf, должна быть не ниже 3.18.2.

## Установка
1) Создать паку build в каталоге с файлом *CmakeLists.txt* и перейти в нее
2) Открыть консоль в данной папке
3) Ввести в консоль команды:

```
cmake .. -DCMAKE_PREFIX_PATH=(путь к библиотекам Protobuf)
cmake --build . --config Debug(либо Release, в зависимости от требований)
```
4) Готово! **Транспортный каталог** установлен на ваше устройство.

## Дальнейшее развитие
* Для ускорения работы **транспортного каталога** возможна реализация поддержки мультипоточности.