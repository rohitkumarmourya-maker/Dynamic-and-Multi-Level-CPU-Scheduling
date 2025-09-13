#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include <algorithm>
#include <string>
#include <list>
#include <map>

using ll = long long;

struct Process {
    int id;
    ll arrival_time;
    int initial_priority;
    ll cpu_burst_1;
    ll io_burst;
    ll cpu_burst_2;
    int type; // 0 for RR, 1 for FCFS

    int current_priority;
    ll remaining_burst_1;
    ll remaining_burst_2;
    ll completion_time = 0;
    ll turnaround_time = 0;
    ll waiting_time = 0;
    ll response_time = -1;
    
    ll time_in_ready_queue = 0;
    ll io_completion_time = -1;
    bool current_burst_is_1 = true;
    ll remaining_quantum = 0;
};

struct GanttEvent {
    ll time;
    std::string event_description;
};

// Custom comparator for priority queue: lower number = higher priority
struct ComparePriority {
    bool operator()(const Process* p1, const Process* p2) {
        if (p1->current_priority != p2->current_priority) {
            return p1->current_priority > p2->current_priority;
        }
        return p1->arrival_time > p2->arrival_time; // FCFS for tie-breaking
    }
};

void print_statistics(const std::vector<Process>& processes, ll total_time, ll total_cpu_busy_time) {
    ll total_wt = 0, total_tat = 0, total_rt = 0;
    std::cout << "\n--- Process Statistics ---\n";
    std::cout << std::left << std::setw(5) << "PID"
              << std::setw(15) << "Waiting Time"
              << std::setw(18) << "Turnaround Time"
              << std::setw(15) << "Response Time" << std::endl;

    for (const auto& p : processes) {
        std::cout << std::left << std::setw(5) << p.id
                  << std::setw(15) << p.waiting_time
                  << std::setw(18) << p.turnaround_time
                  << std::setw(15) << p.response_time << std::endl;
        total_wt += p.waiting_time;
        total_tat += p.turnaround_time;
        total_rt += p.response_time;
    }

    double cpu_utilization = (total_time > 0) ? (static_cast<double>(total_cpu_busy_time) / total_time) * 100.0 : 0.0;

    std::cout << "\n--- Averages & Utilization ---\n";
    std::cout << "Average Waiting Time: " << static_cast<double>(total_wt) / processes.size() << std::endl;
    std::cout << "Average Turnaround Time: " << static_cast<double>(total_tat) / processes.size() << std::endl;
    std::cout << "Average Response Time: " << static_cast<double>(total_rt) / processes.size() << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "CPU Utilization: " << cpu_utilization << "%" << std::endl;
}

void display_gantt_chart(const std::vector<GanttEvent>& chart) {
    std::cout << "\n--- Gantt Chart ---\n";
    for (size_t i = 0; i < chart.size(); ++i) {
        std::cout << "| " << chart[i].event_description << " ";
    }
    std::cout << "|\n";
    
    std::cout << chart[0].time;
    for (size_t i = 1; i < chart.size(); ++i) {
        std::cout << std::setw(chart[i-1].event_description.length() + 3) << chart[i].time;
    }
    std::cout << std::endl;
}


// =========================================================================
// == TASK 1: Preemptive Priority Scheduling with Aging
// =========================================================================
void run_priority_scheduler() {
    int n;
    ll aging_interval_x;
    ll context_switch_time;

    std::cout << "\n--- Task 1: Anti-Starvation Priority Scheduler ---\n";
    std::cout << "Enter number of processes: ";
    std::cin >> n;
    std::cout << "Enter Aging Interval (X): ";
    std::cin >> aging_interval_x;
    std::cout << "Enter Context Switch Time: ";
    std::cin >> context_switch_time;

    std::vector<Process> processes(n);
    std::cout << "Enter process details (Arrival Time, Priority, CPU Burst 1, I/O Burst, CPU Burst 2):\n";
    for (int i = 0; i < n; ++i) {
        processes[i].id = i + 1;
        std::cin >> processes[i].arrival_time >> processes[i].initial_priority 
                 >> processes[i].cpu_burst_1 >> processes[i].io_burst >> processes[i].cpu_burst_2;
        processes[i].current_priority = processes[i].initial_priority;
        processes[i].remaining_burst_1 = processes[i].cpu_burst_1;
        processes[i].remaining_burst_2 = processes[i].cpu_burst_2;
    }
    
    std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });

    std::priority_queue<Process*, std::vector<Process*>, ComparePriority> ready_queue;
    std::list<Process*> blocked_queue;
    std::vector<GanttEvent> gantt_chart;

    ll current_time = 0;
    int completed_processes = 0;
    int process_idx = 0;
    Process* running_process = nullptr;
    Process* last_running_process = nullptr;
    ll total_cpu_busy_time = 0;

    gantt_chart.push_back({0, "Start"});

    while (completed_processes < n) {
        while (process_idx < n && processes[process_idx].arrival_time <= current_time) {
            ready_queue.push(&processes[process_idx]);
            process_idx++;
        }

        for (auto it = blocked_queue.begin(); it != blocked_queue.end();) {
            if ((*it)->io_completion_time <= current_time) {
                ready_queue.push(*it);
                it = blocked_queue.erase(it);
            } else {
                ++it;
            }
        }

        // Aging: Increase priority of waiting processes
        if (aging_interval_x > 0) {
            std::vector<Process*> temp_processes;
            while(!ready_queue.empty()){
                Process* p = ready_queue.top();
                ready_queue.pop();
                if (p != running_process) {
                   p->time_in_ready_queue++;
                   if (p->time_in_ready_queue >= aging_interval_x) {
                       if(p->current_priority > 0) {
                           p->current_priority--; 
                       }
                       p->time_in_ready_queue = 0;
                   }
                }
                temp_processes.push_back(p);
            }
            for(auto p : temp_processes){
                ready_queue.push(p);
            }
        }
        
        Process* next_process = ready_queue.empty() ? nullptr : ready_queue.top();

        if (running_process == nullptr) {
            if (next_process != nullptr) {
                running_process = next_process;
                ready_queue.pop();
                
                if (last_running_process != nullptr) { // Context switch only if CPU was previously busy
                    current_time += context_switch_time;
                    gantt_chart.push_back({current_time, "CS"});
                }
                
                if (running_process->response_time == -1) {
                    running_process->response_time = current_time - running_process->arrival_time;
                }
            }
        } else {
            // Preemption check: new process has strictly higher priority
            if (next_process != nullptr && next_process->current_priority < running_process->current_priority) {
                ready_queue.pop();
                
                current_time += context_switch_time;
                gantt_chart.push_back({current_time, "CS"});

                ready_queue.push(running_process);
                running_process = next_process;

                if (running_process->response_time == -1) {
                    running_process->response_time = current_time - running_process->arrival_time;
                }
            }
        }

        if (running_process != nullptr) {
            std::string current_event = "P" + std::to_string(running_process->id);
            if (gantt_chart.back().event_description != current_event && gantt_chart.back().event_description != "CS") {
                gantt_chart.push_back({current_time, current_event});
            }

            total_cpu_busy_time++;
            if (running_process->current_burst_is_1) {
                running_process->remaining_burst_1--;
                if (running_process->remaining_burst_1 == 0) {
                    running_process->current_burst_is_1 = false;
                    if (running_process->io_burst > 0) {
                        running_process->io_completion_time = current_time + 1 + running_process->io_burst;
                        blocked_queue.push_back(running_process);
                    } else {
                         ready_queue.push(running_process);
                    }
                    last_running_process = running_process;
                    running_process = nullptr;
                }
            } else {
                running_process->remaining_burst_2--;
                if (running_process->remaining_burst_2 == 0) {
                    running_process->completion_time = current_time + 1;
                    running_process->turnaround_time = running_process->completion_time - running_process->arrival_time;
                    running_process->waiting_time = running_process->turnaround_time - running_process->cpu_burst_1 - running_process->cpu_burst_2;
                    completed_processes++;
                    last_running_process = running_process;
                    running_process = nullptr;
                }
            }
        } else {
             if (gantt_chart.back().event_description != "Idle") {
                gantt_chart.push_back({current_time, "Idle"});
            }
        }
        current_time++;
    }
    
    gantt_chart.push_back({current_time, "End"});
    display_gantt_chart(gantt_chart);
    print_statistics(processes, current_time, total_cpu_busy_time);
}

// =========================================================================
// == TASK 2: Multi-Level Queue Scheduler
// =========================================================================
void run_multi_level_scheduler() {
    int n;
    ll rr_time_quantum;
    ll context_switch_time;

    std::cout << "\n--- Task 2: Multi-Level Queue Scheduler ---\n";
    std::cout << "Enter number of processes: ";
    std::cin >> n;
    std::cout << "Enter Time Quantum for RR Queue: ";
    std::cin >> rr_time_quantum;
    std::cout << "Enter Context Switch Time: ";
    std::cin >> context_switch_time;

    std::vector<Process> processes(n);
    std::cout << "Enter process details (Arrival Time, Type (0=FG, 1=BG), CPU Burst 1, I/O Burst, CPU Burst 2):\n";
    for (int i = 0; i < n; ++i) {
        processes[i].id = i + 1;
        std::cin >> processes[i].arrival_time >> processes[i].type 
                 >> processes[i].cpu_burst_1 >> processes[i].io_burst >> processes[i].cpu_burst_2;
        processes[i].remaining_burst_1 = processes[i].cpu_burst_1;
        processes[i].remaining_burst_2 = processes[i].cpu_burst_2;
    }
    
    std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });

    std::queue<Process*> rr_queue;
    std::queue<Process*> fcfs_queue;
    std::list<Process*> blocked_queue;
    std::vector<GanttEvent> gantt_chart;

    ll current_time = 0;
    int completed_processes = 0;
    int process_idx = 0;
    Process* running_process = nullptr;
    Process* last_running_process = nullptr;
    ll total_cpu_busy_time = 0;
    
    gantt_chart.push_back({0, "Start"});

    while(completed_processes < n) {
        while (process_idx < n && processes[process_idx].arrival_time <= current_time) {
            if (processes[process_idx].type == 0) {
                rr_queue.push(&processes[process_idx]);
            } else {
                fcfs_queue.push(&processes[process_idx]);
            }
            process_idx++;
        }

        for (auto it = blocked_queue.begin(); it != blocked_queue.end();) {
            if ((*it)->io_completion_time <= current_time) {
                if ((*it)->type == 0) {
                    rr_queue.push(*it);
                } else {
                    fcfs_queue.push(*it);
                }
                it = blocked_queue.erase(it);
            } else {
                ++it;
            }
        }
        
        // Scheduler logic: RR queue has absolute priority
        if (!rr_queue.empty()) {
            // Preempt FCFS process if RR process is ready
            if (running_process != nullptr && running_process->type == 1) { 
                fcfs_queue.push(running_process);
                running_process = nullptr;
            }
            
            if (running_process == nullptr) {
                running_process = rr_queue.front();
                rr_queue.pop();
                running_process->remaining_quantum = rr_time_quantum;

                if (last_running_process != running_process) {
                     current_time += context_switch_time;
                     gantt_chart.push_back({current_time, "CS"});
                }
                 if (running_process->response_time == -1) {
                    running_process->response_time = current_time - running_process->arrival_time;
                }
            }
        } 
        // Only run FCFS if RR queue is empty and CPU is idle
        else if (!fcfs_queue.empty() && running_process == nullptr) { 
            running_process = fcfs_queue.front();
            fcfs_queue.pop();
            
            if (last_running_process != running_process) {
                current_time += context_switch_time;
                gantt_chart.push_back({current_time, "CS"});
            }
            if (running_process->response_time == -1) {
                running_process->response_time = current_time - running_process->arrival_time;
            }
        }
        
        if (running_process != nullptr) {
            std::string current_event = "P" + std::to_string(running_process->id);
            if (gantt_chart.back().event_description != current_event && gantt_chart.back().event_description != "CS") {
                gantt_chart.push_back({current_time, current_event});
            }
            total_cpu_busy_time++;

            if (running_process->current_burst_is_1) running_process->remaining_burst_1--;
            else running_process->remaining_burst_2--;
            
            if(running_process->type == 0) running_process->remaining_quantum--;

            ll current_burst_remaining = running_process->current_burst_is_1 ? running_process->remaining_burst_1 : running_process->remaining_burst_2;

            if (current_burst_remaining == 0) { // Burst completion
                if (running_process->current_burst_is_1) {
                    running_process->current_burst_is_1 = false;
                    if(running_process->io_burst > 0) {
                        running_process->io_completion_time = current_time + 1 + running_process->io_burst;
                        blocked_queue.push_back(running_process);
                    } else {
                        if(running_process->type == 0) rr_queue.push(running_process);
                        else fcfs_queue.push(running_process);
                    }
                } else { // Process completion
                    running_process->completion_time = current_time + 1;
                    running_process->turnaround_time = running_process->completion_time - running_process->arrival_time;
                    running_process->waiting_time = running_process->turnaround_time - running_process->cpu_burst_1 - running_process->cpu_burst_2;
                    completed_processes++;
                }
                last_running_process = running_process;
                running_process = nullptr;
            } 
            else if (running_process->type == 0 && running_process->remaining_quantum == 0) { // Quantum expiry
                rr_queue.push(running_process);
                last_running_process = running_process;
                running_process = nullptr;
            }
        } else {
            if (gantt_chart.back().event_description != "Idle") {
                gantt_chart.push_back({current_time, "Idle"});
            }
        }
        current_time++;
    }
    gantt_chart.push_back({current_time, "End"});

    display_gantt_chart(gantt_chart);
    print_statistics(processes, current_time, total_cpu_busy_time);
}


int main() {
    int choice;
    std::cout << "Select the scheduler to run:" << std::endl;
    std::cout << "1. Anti-Starvation Priority Scheduler (Task 1)" << std::endl;
    std::cout << "2. Multi-Level Queue Scheduler (Task 2)" << std::endl;
    std::cout << "Enter your choice: ";
    std::cin >> choice;

    switch (choice) {
        case 1:
            run_priority_scheduler();
            break;
        case 2:
            run_multi_level_scheduler();
            break;
        default:
            std::cout << "Invalid choice. Exiting." << std::endl;
            break;
    }

    return 0;
}