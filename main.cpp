#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <boost/tokenizer.hpp>
#include <boost/assign.hpp>
#include <typeinfo>
#include <random>
#include <algorithm>
#include <math.h>
#include <chrono>

// !! check to make sure sorting doesn't fuck shit up
// !! check to make sure
using namespace std;
using namespace boost;

const int NUM_SIM_RUNS = 2; // will eventually make a very big number
const int NUM_LV = 5;
const int NUM_Operators = 10;
const int NUM_Payloads = 100;
const int NUM_Years = 3;
const int START_YEAR = 2018;

std::random_device rd;
std::mt19937_64 engine(rd());
std::uniform_real_distribution<double> generator(0, 1);

enum Orbits {GEO, GTO, LEO, SSO};

struct capacity_by_orbit {
    Orbits orbit;
    int capacity;
};

struct operator_priority {
    string operator_name;
    int index;
    int value;
};

struct launch_vehicle {
    string name;
    int index;
    capacity_by_orbit  capacities [4];
    int max_throughput;
    int current_throughput[NUM_Years] = {0,0,0};
    operator_priority operator_priorities [NUM_Operators];

    int overage(int zero_based_year) {
        return max_throughput-current_throughput[zero_based_year];
    }
};

struct launch_vehicle_preference {
    int launch_vehicle_index;
    float value;
};
struct operator_payload_struct {
    string name;
    int index;
    launch_vehicle_preference launch_vehicle_preferences [NUM_LV];
    float likelihood_picking_2nd_choice;
};

struct payload {
    string name;
    int mass;
    int index;
    operator_payload_struct operatorPayload;
    launch_vehicle_preference launch_vehicle_preferences [NUM_LV];
    Orbits destination;
    int launch_year;
    launch_vehicle launch_vehicle_selection;
    float lv_priority_for_operator;
    bool can_be_bumped = 1;
};

std::chrono::time_point<std::chrono::high_resolution_clock> read_timer() {
    auto get_time = std::chrono::high_resolution_clock::now();
    return get_time;
}

void read_file(string filename, vector<vector< string >> &vec_of_file_as_strings) {
    string data(filename);
    ifstream in(data.c_str());
    if (!in.is_open()) {
        cout << "file could not be opened" <<endl;
        return;
    }
    typedef tokenizer< escaped_list_separator<char> > Tokenizer;
    vector< string > vec;
    string line;
    while (getline(in,line)) {
        Tokenizer tok(line);
        vec.assign(tok.begin(),tok.end());
        vec_of_file_as_strings.push_back(vec);
    }
};

void init_launch_vehicles_vec(vector<vector< string >> &vec_of_file_as_strings, vector<launch_vehicle> &launch_vehicles){
    for (int lv_line = 1; lv_line<=NUM_LV; ++lv_line) {
        launch_vehicles[lv_line-1].name = vec_of_file_as_strings[lv_line][0];
        launch_vehicles[lv_line-1].index = lv_line-1;
        launch_vehicles[lv_line-1].capacities[0].orbit = GEO;
        launch_vehicles[lv_line-1].capacities[0].capacity = atoi(vec_of_file_as_strings[lv_line][1].c_str());
        launch_vehicles[lv_line-1].capacities[1].orbit = GTO;
        launch_vehicles[lv_line-1].capacities[1].capacity = atoi(vec_of_file_as_strings[lv_line][2].c_str());
        launch_vehicles[lv_line-1].capacities[2].orbit = LEO;
        launch_vehicles[lv_line-1].capacities[2].capacity = atoi(vec_of_file_as_strings[lv_line][3].c_str());
        launch_vehicles[lv_line-1].capacities[3].orbit = SSO;
        launch_vehicles[lv_line-1].capacities[3].capacity = atoi(vec_of_file_as_strings[lv_line][4].c_str());
        launch_vehicles[lv_line-1].max_throughput = atoi(vec_of_file_as_strings[lv_line][5].c_str());
        launch_vehicles[lv_line-1].operator_priorities[0].value = atoi(vec_of_file_as_strings[lv_line][6].c_str());
        launch_vehicles[lv_line-1].operator_priorities[1].value = atoi(vec_of_file_as_strings[lv_line][7].c_str());
        launch_vehicles[lv_line-1].operator_priorities[2].value = atoi(vec_of_file_as_strings[lv_line][8].c_str());
        launch_vehicles[lv_line-1].operator_priorities[3].value = atoi(vec_of_file_as_strings[lv_line][9].c_str());
        launch_vehicles[lv_line-1].operator_priorities[4].value = atoi(vec_of_file_as_strings[lv_line][10].c_str());
        launch_vehicles[lv_line-1].operator_priorities[5].value = atoi(vec_of_file_as_strings[lv_line][11].c_str());
        launch_vehicles[lv_line-1].operator_priorities[6].value = atoi(vec_of_file_as_strings[lv_line][12].c_str());
        launch_vehicles[lv_line-1].operator_priorities[7].value = atoi(vec_of_file_as_strings[lv_line][13].c_str());
        launch_vehicles[lv_line-1].operator_priorities[8].value = atoi(vec_of_file_as_strings[lv_line][14].c_str());
        launch_vehicles[lv_line-1].operator_priorities[9].value = atoi(vec_of_file_as_strings[lv_line][15].c_str());
    }
    vec_of_file_as_strings.clear();
};

void init_operators_vec(vector<vector< string >> &vec_of_file_as_strings, vector<operator_payload_struct> &operators,
                        vector<launch_vehicle> &launch_vehicles){
    for (int o_line = 1; o_line<=NUM_Operators; ++o_line) {
        operators[o_line-1].name = vec_of_file_as_strings[o_line][0];
        operators[o_line-1].index = o_line-1;
        operators[o_line-1].launch_vehicle_preferences[0].launch_vehicle_index = launch_vehicles[0].index;
        operators[o_line-1].launch_vehicle_preferences[1].launch_vehicle_index = launch_vehicles[1].index;
        operators[o_line-1].launch_vehicle_preferences[2].launch_vehicle_index = launch_vehicles[2].index;
        operators[o_line-1].launch_vehicle_preferences[3].launch_vehicle_index = launch_vehicles[3].index;
        operators[o_line-1].launch_vehicle_preferences[4].launch_vehicle_index = launch_vehicles[4].index;
        operators[o_line-1].launch_vehicle_preferences[0].value = strtof(vec_of_file_as_strings[o_line][1].c_str(),0);
        operators[o_line-1].launch_vehicle_preferences[1].value = strtof(vec_of_file_as_strings[o_line][2].c_str(),0);
        operators[o_line-1].launch_vehicle_preferences[2].value = strtof(vec_of_file_as_strings[o_line][3].c_str(),0);
        operators[o_line-1].launch_vehicle_preferences[3].value = strtof(vec_of_file_as_strings[o_line][4].c_str(),0);
        operators[o_line-1].launch_vehicle_preferences[4].value = strtof(vec_of_file_as_strings[o_line][5].c_str(),0);
        operators[o_line-1].likelihood_picking_2nd_choice = strtof(vec_of_file_as_strings[o_line][6].c_str(),0);
    }
    vec_of_file_as_strings.clear();
};

void init_payloads_vec(vector<vector< string >> &vec_of_file_as_strings, vector<payload> &payloads,
                       vector<operator_payload_struct> &operators, vector<launch_vehicle> &launch_vehicles){
    for (int p_line = 1; p_line <=NUM_Payloads; ++p_line) {
        payloads[p_line-1].name = vec_of_file_as_strings[p_line][0];
        payloads[p_line-1].mass = atoi(vec_of_file_as_strings[p_line][1].c_str());
        payloads[p_line-1].index = p_line-1;
        string csv_operator_name = vec_of_file_as_strings[p_line][2];
        payloads[p_line-1].operatorPayload = find_if(std::begin(operators), std::end(operators),
                                                    [&csv_operator_name](const operator_payload_struct& operatorPayload) {
            return operatorPayload.name == csv_operator_name;
        })[0];
        if (vec_of_file_as_strings[p_line][3] == "GEO") { payloads[p_line-1].destination = GEO;}
        else if (vec_of_file_as_strings[p_line][3] == "GTO") { payloads[p_line-1].destination = GTO; }
        else if (vec_of_file_as_strings[p_line][3] == "LEO") { payloads[p_line-1].destination = LEO; }
        else if (vec_of_file_as_strings[p_line][3] == "SSO") { payloads[p_line-1].destination = SSO; }

        payloads[p_line-1].launch_vehicle_preferences[0].launch_vehicle_index = launch_vehicles[0].index;
        payloads[p_line-1].launch_vehicle_preferences[1].launch_vehicle_index = launch_vehicles[1].index;
        payloads[p_line-1].launch_vehicle_preferences[2].launch_vehicle_index = launch_vehicles[2].index;
        payloads[p_line-1].launch_vehicle_preferences[3].launch_vehicle_index = launch_vehicles[3].index;
        payloads[p_line-1].launch_vehicle_preferences[4].launch_vehicle_index = launch_vehicles[4].index;
        payloads[p_line-1].launch_year = atoi(vec_of_file_as_strings[p_line][4].c_str());
    }
    vec_of_file_as_strings.clear();
};

int can_vehicle_take_payload(launch_vehicle &launch_vehicle_1, payload &payload_1) {
    Orbits payload_orbit = payload_1.destination;
    int lv_capacity = launch_vehicle_1.capacities[payload_orbit].capacity;
    if (lv_capacity >= payload_1.mass) { return 1; }
    else { return 0; }
};

void get_payload_operator_launch_preferences(int lv_num, vector<operator_payload_struct> &operators,
                                             payload &payload_1) {
    operator_payload_struct my_operator = payload_1.operatorPayload;
    payload_1.launch_vehicle_preferences[lv_num].value = my_operator.launch_vehicle_preferences[lv_num].value;
};

float sum_preferences(payload &payload_1) {
    float preferences_sum = 0.0;
    for (int lv_num = 0; lv_num<NUM_LV; ++lv_num) {
        preferences_sum = preferences_sum + payload_1.launch_vehicle_preferences[lv_num].value;
    }
    return preferences_sum;
}

void select_lv_and_add_to_summary_array(vector<launch_vehicle> &launch_vehicles, payload &payload_1, float lv_scaled[NUM_LV]) {
    float my_rand = generator(engine);

    int zero_based_year = payload_1.launch_year-START_YEAR;
    float rolling_sum = 0;
    for (int lv_num = 0; lv_num < NUM_LV; ++lv_num) {
        if(my_rand <= lv_scaled[lv_num]+rolling_sum) {
            payload_1.launch_vehicle_selection = launch_vehicles[lv_num];
            launch_vehicles[lv_num].current_throughput[zero_based_year] +=1;
            return;
        }
        rolling_sum = rolling_sum + lv_scaled[lv_num];
    }
    payload_1.launch_vehicle_selection = launch_vehicles[4];
    launch_vehicles[4].current_throughput[zero_based_year] +=1;
    return;
}

void randomly_select_launch_vehicle(vector<launch_vehicle> &launch_vehicles, payload &payload_1) {
    float lv_scaled[NUM_LV];
    float preferences_sum = sum_preferences(payload_1);
    //change the scale of the lv preferences so that it goes from 0-1
    for (int lv_num = 0; lv_num<NUM_LV; ++lv_num) {
        lv_scaled[lv_num] = payload_1.launch_vehicle_preferences[lv_num].value/preferences_sum;
    }

    select_lv_and_add_to_summary_array(launch_vehicles,payload_1,lv_scaled);
}

void select_launch_vehicles_initially(vector<launch_vehicle> &launch_vehicles,
                                      vector<operator_payload_struct> &operators, vector<payload> &payloads) {
    for (int i = 0; i < NUM_LV; ++i) {
        std::fill(std::begin(launch_vehicles[i].current_throughput), std::end(launch_vehicles[i].current_throughput), 0);
    }
    for(int payload_num = 0; payload_num<NUM_Payloads; payload_num++) {
        for (int lv_num = 0; lv_num<NUM_LV; lv_num++) {
            int launchable = can_vehicle_take_payload(launch_vehicles[lv_num],payloads[payload_num]);
            if (launchable) {
                get_payload_operator_launch_preferences(lv_num,operators,payloads[payload_num]);
            }
        }
        randomly_select_launch_vehicle(launch_vehicles,payloads[payload_num]);
    }

};

bool is_lower_priority(const payload& x, const payload& y) {return x.lv_priority_for_operator < y.lv_priority_for_operator;};

void collect_lv_priorities_for_operators(launch_vehicle &launch_vehicle_1, vector<payload> &payloads,
                                         int zero_based_year, vector<payload> (&relevant_payloads), int lv_num) {
    for (int p_num = 0; p_num < launch_vehicle_1.current_throughput[zero_based_year]; ++p_num) {
        if (!relevant_payloads[p_num].can_be_bumped) {
            relevant_payloads[p_num].lv_priority_for_operator = 0 + generator(engine);
        }
        else {
            int operator_index_for_payload = relevant_payloads[p_num].operatorPayload.index;
            float priority_num = launch_vehicle_1.operator_priorities[operator_index_for_payload].value + generator(engine);
            relevant_payloads[p_num].lv_priority_for_operator = priority_num;
        }
    }
}

void add_n_lowest_priority_payloads_to_bump_list(vector<launch_vehicle> &launch_vehicles,vector<payload> &payloads,vector<payload> &payload_batch,
                                                 vector<int> &bumped_payload_indices, int max_minus_current_throughput_single,
                                                 int lv_num, int zero_based_year) {
    for (int overflow_num = 0; overflow_num < -max_minus_current_throughput_single; ++overflow_num) {
        int bumped_payload_index = payload_batch[overflow_num].index; //dies here
        bumped_payload_indices.push_back(bumped_payload_index);
        payload_batch.front() = move(payload_batch.back());
        payload_batch.pop_back();
        launch_vehicles[lv_num].current_throughput[zero_based_year]--;
    }
}

void pick_bumped_payloads_in_a_year(vector<launch_vehicle> &launch_vehicles, vector<payload> &payloads, int zero_based_year,
                                    vector<int> &bumped_payload_indices) {
    for (int lv_num = 0; lv_num < NUM_LV; ++lv_num) {
        int max_minus_current_throughput = launch_vehicles[lv_num].overage(zero_based_year);
        if (max_minus_current_throughput <0) {
            vector <payload> relevant_payloads;
            for (int i=0; i < payloads.size(); ++i) {
                if (payloads[i].launch_vehicle_selection.index == launch_vehicles[lv_num].index && payloads[i].launch_year-START_YEAR == zero_based_year) {
                    relevant_payloads.push_back(payloads[i]);
                }
            }
            collect_lv_priorities_for_operators(launch_vehicles[lv_num],payloads,zero_based_year,relevant_payloads, lv_num);
            sort(relevant_payloads.begin(),relevant_payloads.end(),is_lower_priority);
            add_n_lowest_priority_payloads_to_bump_list(launch_vehicles, payloads,relevant_payloads,bumped_payload_indices,max_minus_current_throughput, lv_num, zero_based_year);
        }
    }
}

bool is_higher_preference(const launch_vehicle_preference& x, const launch_vehicle_preference& y) {return x.value > y.value;};

int fly_on_2nd_choice(vector<launch_vehicle> &launch_vehicles,vector<payload> &payloads, int bumped_payload_index, int zero_based_year) {
    if (payloads[bumped_payload_index].operatorPayload.likelihood_picking_2nd_choice > generator(engine)) {
        launch_vehicle_preference copy_of_launch_pref_array [NUM_LV];
        copy(begin(payloads[bumped_payload_index].launch_vehicle_preferences), end(payloads[bumped_payload_index].launch_vehicle_preferences),begin(copy_of_launch_pref_array));
        sort(begin(copy_of_launch_pref_array),end(copy_of_launch_pref_array),is_higher_preference);
        int top_index = copy_of_launch_pref_array[0].launch_vehicle_index;
        int current_index = payloads[bumped_payload_index].launch_vehicle_selection.index;
        if (top_index == current_index) {
            top_index = copy_of_launch_pref_array[1].launch_vehicle_index;
        }
        if (launch_vehicles[top_index].overage(zero_based_year) > 0) {
            int launchable = can_vehicle_take_payload(launch_vehicles[top_index],payloads[bumped_payload_index]);
            if (launchable) {
                return top_index;
            }
        }
    }
    return -1;
};

void change_payload_vehicle(vector<launch_vehicle> &launch_vehicles, vector<payload> &payloads,int &bumped_payload_index,
                            int index_2nd_choice, int zero_based_year) {
    payloads[bumped_payload_index].launch_vehicle_selection = launch_vehicles[index_2nd_choice];
    launch_vehicles[index_2nd_choice].current_throughput[zero_based_year] +=1;
};

void change_payload_launch_year(vector<launch_vehicle> &launch_vehicles,vector<payload> &payloads,
                                int &bumped_payload_index, int zero_based_year) {
    payloads[bumped_payload_index].launch_year += 1;
    payloads[bumped_payload_index].can_be_bumped = 0;
    if (zero_based_year+1 < NUM_Years) {
        launch_vehicles[payloads[bumped_payload_index].launch_vehicle_selection.index].current_throughput[zero_based_year+1] +=1;
    }
};

void handle_bumped_payloads(vector<launch_vehicle> &launch_vehicles,vector<payload> &payloads,
                            vector<int> bumped_payload_indices,
                            int zero_based_year) {
    for (int p_num = 0; p_num<bumped_payload_indices.size(); ++p_num) {
        int will_payload_fly_on_2nd_choice = fly_on_2nd_choice(launch_vehicles, payloads, bumped_payload_indices[p_num], zero_based_year);
        if (will_payload_fly_on_2nd_choice >= 0) {
            change_payload_vehicle(launch_vehicles, payloads, bumped_payload_indices[p_num],will_payload_fly_on_2nd_choice,zero_based_year);
        }
        else {
            change_payload_launch_year(launch_vehicles, payloads, bumped_payload_indices[p_num], zero_based_year);
        }
    }
};

void adjust_launch_vehicle_selections(vector<launch_vehicle> &launch_vehicles,
                                      vector<operator_payload_struct> &operators, vector<payload> &payloads) {
    vector<int> bumped_payload_indices;
    for (int zero_based_year = 0; zero_based_year < NUM_Years; ++zero_based_year) {
        bumped_payload_indices.clear();
        pick_bumped_payloads_in_a_year(launch_vehicles, payloads, zero_based_year,
                                       bumped_payload_indices);
        handle_bumped_payloads(launch_vehicles,payloads, bumped_payload_indices, zero_based_year);
    }
};

void collect_launch_statistics(vector<launch_vehicle> &launch_vehicles, int (&average_launch_stats)[NUM_Years][NUM_LV]) {
    for (int year_num = 0; year_num < NUM_Years; ++year_num) {
        for (int lv_num = 0; lv_num < NUM_LV; ++lv_num) {
            average_launch_stats[year_num][lv_num]+= launch_vehicles[lv_num].current_throughput[year_num];
        }
    }
};

int idiv_ceil ( int numerator, int denominator ) //credit: https://stackoverflow.com/questions/17005364/dividing-two-integers-and-rounding-up-the-result-without-using-floating-point
{
    return numerator / denominator
           + (((numerator < 0) ^ (denominator > 0)) && (numerator%denominator));
}

void average_launch_statistics(vector<launch_vehicle> &launch_vehicles, int (&average_launch_stats)[NUM_Years][NUM_LV]) {
    cout << "in average_launch_statistics" << endl;
    for (int year_num = 0; year_num < NUM_Years; ++year_num) {
        cout << "Year: " << year_num+START_YEAR << "\n";
        for (int lv_num = 0; lv_num < NUM_LV; ++lv_num) {
            average_launch_stats[year_num][lv_num]=  idiv_ceil(average_launch_stats[year_num][lv_num],NUM_SIM_RUNS);
            cout << launch_vehicles[lv_num].name << " : " << average_launch_stats[year_num][lv_num] << "  ";
        }
        cout << "\n";
    }
    cout << "\n";
};

void clear_generated_variables(vector<launch_vehicle> &launch_vehicles, vector<payload> &payloads) {
    for (int lv_num = 0; lv_num <NUM_LV; ++lv_num) {
        for (int year_num = 0; year_num <NUM_Years; ++year_num)
        launch_vehicles[lv_num].current_throughput[year_num] = 0;
    }
}

void run_simulation(string launch_vehicles_file, string operators_file, string payloads_file,
                    vector<vector< string >> &vec_of_file_as_strings, vector<launch_vehicle> &launch_vehicles,
                    vector<operator_payload_struct> &operators, vector<payload> &payloads){
    read_file(launch_vehicles_file, vec_of_file_as_strings);
    init_launch_vehicles_vec(vec_of_file_as_strings, launch_vehicles);
    read_file(operators_file, vec_of_file_as_strings);
    init_operators_vec(vec_of_file_as_strings, operators, launch_vehicles);
    read_file(payloads_file, vec_of_file_as_strings);
    init_payloads_vec(vec_of_file_as_strings, payloads,operators, launch_vehicles);
    int average_launch_stats[NUM_Years][NUM_LV] = {{0}};
    for (int run_num = 0; run_num < NUM_SIM_RUNS; run_num++) {
        select_launch_vehicles_initially(launch_vehicles, operators, payloads);
        adjust_launch_vehicle_selections(launch_vehicles, operators, payloads);
        collect_launch_statistics(launch_vehicles, average_launch_stats);
        clear_generated_variables(launch_vehicles,payloads);
    }
    average_launch_statistics(launch_vehicles, average_launch_stats);
};

int main() {

    vector<launch_vehicle> launch_vehicles(static_cast<unsigned long>(NUM_LV));
    vector<operator_payload_struct> operators(static_cast<unsigned long>(NUM_Operators));
    vector<payload> payloads(static_cast<unsigned long>(NUM_Payloads));

    vector<vector< string >> vec_of_file_as_strings;

    string launch_vehicles_file = "../LaunchForecastDummyData - LaunchVehicles.csv";
    string operators_file = "../LaunchForecastDummyData - Operators.csv";
    string payloads_file = "../LaunchForecastDummyData - Payloads.csv";

    auto timer = read_timer();
    run_simulation(launch_vehicles_file, operators_file, payloads_file, vec_of_file_as_strings, launch_vehicles, operators, payloads);
    auto simulation_time =
            std::chrono::duration<double, std::micro>(read_timer() - timer).count() /
            1e6;
    std::cout << "simulation time = " << simulation_time << " seconds";

    return 0;
}