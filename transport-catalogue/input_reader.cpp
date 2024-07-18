#include <algorithm>
#include "input_reader.h"
#include <iostream>
#include <cassert>
#include <iterator>
#include <string>
#include <utility>

using namespace std;

namespace transport_catalogue {
    namespace input_reader {

        /**
         * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
         */
        Coordinates ParseCoordinates(string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return {nan, nan};
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);
            auto comma2 = str.find(',', comma + 1);

            double lat = stod(string(str.substr(not_space, comma - not_space)));
            double lng = stod(string(str.substr(not_space2, comma2 - not_space2)));

            return {lat, lng};
        }

        /**
         * Удаляет пробелы в начале и конце строки
         */
        string_view Trim(string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        /**
         * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
         */
        vector<string_view> Split(string_view string, char delim) {
            vector<string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }

        /**
         * Парсит маршрут.
         * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
         * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
         */
        vector<string_view> ParseRoute(string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            vector<string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return {string(line.substr(0, space_pos)),
                    string(line.substr(not_space, colon_pos - not_space)),
                    string(line.substr(colon_pos + 1))};
        }

        void InputReader::ParseLine(string_view line) {
            auto command_description = ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(move(command_description));
            }
        }

        pair<string_view, size_t> ParseDistance(const string_view& data) {
            auto start_pos = data.find_first_not_of(' ');
            auto m_pos = data.find('m', start_pos);
            size_t distance = stoull(static_cast<string>(data.substr(start_pos, m_pos - start_pos)));
            auto to_pos = data.find_first_not_of(' ', m_pos + 1);
            string_view stop_name = data.substr(data.find_first_not_of(' ', to_pos + 2));
            return {stop_name, distance};
        }

        void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& transport_catalogue) {
            sort(commands_.begin(), commands_.end(), [](const auto& lhs, const auto& rhs) {
                return lhs.command > rhs.command;
            });
            vector<const CommandDescription*> buffer;

            for (const auto& command : commands_) {
                if (GetQueryType(command.command) == QueryType::STOP) {
                    buffer.push_back(&command);
                    Coordinates coordinates = ParseCoordinates(command.description);
                    transport_catalogue.AddStop(command.id, move(coordinates));
                } else if (GetQueryType(command.command) == QueryType::BUS) {
                    auto stops = ParseRoute(command.description);
                    transport_catalogue.AddBus(command.id, stops);
                } else {
                    cout << "Unknown command:"s << command.command << endl;
                }
            }
            for (const auto& command : buffer) {
                auto data = Split(command->description, ',');
                if (data.size() > 2) {
                    for (size_t i = 2; i < data.size(); ++i) {
                        auto distance = ParseDistance(data[i]);
                        transport_catalogue.SetDistance(transport_catalogue.GetStop(command->id), transport_catalogue.GetStop(distance.first), distance.second);
                    }
                }
            }
        }

        QueryType GetQueryType(string_view request) {
            auto space_index = request.find(' ');
            string_view command = request.substr(0, space_index);
            if (command == "Bus") {
                return QueryType::BUS;
            } else if (command == "Stop") {
                return QueryType::STOP;
            } else {
                return QueryType::WTF;
            }
        }

    } // end of namespace input_reader
} // end of namespace transport_catalogue