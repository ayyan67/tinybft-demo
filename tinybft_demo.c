#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h>  // For _getch() on Windows

// Configuration
#define NUM_REPLICAS 4
#define FAULTY_THRESHOLD 1  // f value (can tolerate up to f Byzantine faults)
#define MAX_KEY_SIZE 32
#define MAX_VALUE_SIZE 256
#define MAX_KEYS 10

// PBFT message types for protocol demonstration
typedef enum {
    MSG_REQUEST,      // Client request
    MSG_PRE_PREPARE,  // Primary assigns sequence number
    MSG_PREPARE,      // Replicas acknowledge pre-prepare
    MSG_COMMIT,       // Replicas commit to the request
    MSG_REPLY         // Reply to client
} message_type_t;

// Key-value pair
typedef struct {
    char key[MAX_KEY_SIZE];
    char value[MAX_VALUE_SIZE];
    bool used;
} kv_pair_t;

// Replica state
typedef struct {
    int id;
    int view;
    int seq_num;
    bool is_primary;
    bool is_faulty;
    kv_pair_t kv_store[MAX_KEYS];
} replica_t;

// Global state
replica_t replicas[NUM_REPLICAS];
int current_seq = 0;

// Function declarations
void initialize_system(void);
void set_replica_faulty(int replica_id, bool faulty);
bool is_primary(int replica_id);
int get_primary_for_view(int view);
void process_command(const char* command);
void execute_put_command(const char* key, const char* value);
void execute_get_command(const char* key);
void simulate_request_phase(const char* key, const char* value);
void simulate_pre_prepare_phase(const char* key, const char* value);
void simulate_prepare_phase(void);
void simulate_commit_phase(void);
void simulate_execute_phase(const char* key, const char* value);
void update_kv_store(int replica_id, const char* key, const char* value);
void display_status(void);
void display_key_value_stores(void);
void display_memory_usage(void);
void clear_screen(void);
void print_header(const char* title);
void wait_for_key(void);

int main() {
    // Seed random number generator
    srand((unsigned int)time(NULL));
    
    // Initialize system
    initialize_system();
    
    char command[512];
    
    // Welcome message
    clear_screen();
    printf("\n");
    printf("+----------------------------------------------------------+\n");
    printf("|                    TINYBFT DEMO                          |\n");
    printf("|    Byzantine Fault-Tolerant Replication for              |\n");
    printf("|         Highly Resource-Constrained Devices              |\n");
    printf("+----------------------------------------------------------+\n\n");
    printf("This demo illustrates the PBFT protocol with 4 replicas (3f+1 where f=1).\n");
    printf("It demonstrates Byzantine fault tolerance and static memory allocation.\n\n");
    printf("Press any key to start...");
    _getch();
    
    while (1) {
        clear_screen();
        printf("+----------------------------------------------------------+\n");
        printf("|                    TINYBFT DEMO                          |\n");
        printf("+----------------------------------------------------------+\n\n");
        
        // Display replica status
        printf("=== REPLICA STATUS ===\n");
        printf("%-10s %-10s %-15s %-10s\n", "REPLICA", "ROLE", "STATUS", "SEQ_NUM");
        printf("----------------------------------------------\n");
        
        for (int i = 0; i < NUM_REPLICAS; i++) {
            printf("%-10d %-10s %-15s %-10d\n", 
                   i, 
                   is_primary(i) ? "PRIMARY" : "BACKUP", 
                   replicas[i].is_faulty ? "FAULTY" : "CORRECT", 
                   replicas[i].seq_num);
        }
        
        // Display key-value stores
        printf("\n=== KEY-VALUE STORE CONTENTS ===\n");
        display_key_value_stores();
        
        // Display menu
        printf("\n=== AVAILABLE COMMANDS ===\n");
        printf("1. PUT <key> <value>  - Add/update key-value pair\n");
        printf("2. GET <key>          - Retrieve value for key\n");
        printf("3. FAULT <replica>    - Toggle fault status of replica\n");
        printf("4. PROCESS            - Simulate PBFT protocol phases\n");
        printf("5. STATUS             - Show detailed replica status\n");
        printf("6. MEMORY             - Show memory analysis\n");
        printf("7. CLEAR              - Clear the screen\n");
        printf("8. QUIT               - Exit the demo\n");
        
        // Get user command
        printf("\nEnter command: ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        // Remove newline
        command[strcspn(command, "\n")] = '\0';
        
        // Process command
        if (strlen(command) > 0) {
            process_command(command);
        }
    }
    
    return 0;
}

// Initialize system
void initialize_system() {
    // Initialize replicas
    for (int i = 0; i < NUM_REPLICAS; i++) {
        replicas[i].id = i;
        replicas[i].view = 0;
        replicas[i].seq_num = 0;
        replicas[i].is_primary = (i == get_primary_for_view(0));
        replicas[i].is_faulty = false;
        
        // Clear key-value store
        memset(replicas[i].kv_store, 0, sizeof(replicas[i].kv_store));
    }
    
    // Initialize sequence number
    current_seq = 0;
}

// Set replica faulty status
void set_replica_faulty(int replica_id, bool faulty) {
    if (replica_id >= 0 && replica_id < NUM_REPLICAS) {
        replicas[replica_id].is_faulty = faulty;
    }
}

// Check if replica is primary
bool is_primary(int replica_id) {
    return replicas[replica_id].is_primary;
}

// Get primary for view
int get_primary_for_view(int view) {
    return view % NUM_REPLICAS;
}

// Process user command
void process_command(const char* command) {
    char cmd[32];
    char arg1[256];
    char arg2[256];
    
    // Parse command
    if (sscanf(command, "%31s %255s %255s", cmd, arg1, arg2) >= 1) {
        if (strcasecmp(cmd, "PUT") == 0 && arg1[0] != '\0' && arg2[0] != '\0') {
            execute_put_command(arg1, arg2);
        } else if (strcasecmp(cmd, "GET") == 0 && arg1[0] != '\0') {
            execute_get_command(arg1);
        } else if (strcasecmp(cmd, "FAULT") == 0 && arg1[0] != '\0') {
            int replica = atoi(arg1);
            if (replica >= 0 && replica < NUM_REPLICAS) {
                set_replica_faulty(replica, !replicas[replica].is_faulty);
                printf("Replica %d is now %s\n", replica, 
                       replicas[replica].is_faulty ? "FAULTY" : "CORRECT");
                wait_for_key();
            } else {
                printf("Invalid replica ID\n");
                wait_for_key();
            }
        } else if (strcasecmp(cmd, "PROCESS") == 0) {
            clear_screen();
            print_header("PBFT PROTOCOL SIMULATION");
            printf("Simulating the PBFT protocol flow with sample data\n\n");
            
            // Generate a unique key for this simulation
            char demo_key[32], demo_value[32];
            sprintf(demo_key, "key-%d", rand() % 1000);
            sprintf(demo_value, "value-%d", rand() % 1000);
            
            current_seq++;
            
            printf("=== SIMULATING PBFT PROTOCOL PHASES ===\n");
            printf("Operation: PUT %s=%s\n", demo_key, demo_value);
            printf("Press a key after each phase to continue...\n\n");
            wait_for_key();
            
            // Phase 1: Request
            printf("1. CLIENT REQUEST PHASE:\n");
            printf("   Client sends request to primary (Replica %d): PUT %s=%s\n", 
                   get_primary_for_view(0), demo_key, demo_value);
            wait_for_key();
            
            // Phase 2: Pre-prepare
            printf("2. PRE-PREPARE PHASE:\n");
            printf("   Primary (Replica %d) assigns sequence number %d\n", get_primary_for_view(0), current_seq);
            printf("   Primary broadcasts PRE-PREPARE to all replicas\n");
            wait_for_key();
            
            // Phase 3: Prepare
            printf("3. PREPARE PHASE:\n");
            for (int i = 0; i < NUM_REPLICAS; i++) {
                if (!replicas[i].is_faulty) {
                    printf("   Replica %d broadcasts PREPARE message\n", i);
                } else {
                    printf("   Replica %d (FAULTY) might send corrupt PREPARE or none at all\n", i);
                }
            }
            
            int valid_prepares = 0;
            for (int i = 0; i < NUM_REPLICAS; i++) {
                if (!replicas[i].is_faulty) {
                    valid_prepares++;
                }
            }
            
            printf("\n   Total valid PREPARE messages: %d\n", valid_prepares);
            
            if (valid_prepares >= 2 * FAULTY_THRESHOLD + 1) {
                printf("   Each replica receives 2f+1=%d valid PREPAREs (prepare certificate)\n", 
                      2 * FAULTY_THRESHOLD + 1);
            } else {
                printf("   Not enough valid PREPAREs for certificate (%d needed)\n", 
                      2 * FAULTY_THRESHOLD + 1);
            }
            wait_for_key();
            
            // Phase 4: Commit
            printf("4. COMMIT PHASE:\n");
            for (int i = 0; i < NUM_REPLICAS; i++) {
                if (!replicas[i].is_faulty) {
                    printf("   Replica %d broadcasts COMMIT message\n", i);
                } else {
                    printf("   Replica %d (FAULTY) might send corrupt COMMIT or none at all\n", i);
                }
            }
            
            int valid_commits = 0;
            for (int i = 0; i < NUM_REPLICAS; i++) {
                if (!replicas[i].is_faulty) {
                    valid_commits++;
                }
            }
            
            printf("\n   Total valid COMMIT messages: %d\n", valid_commits);
            
            if (valid_commits >= 2 * FAULTY_THRESHOLD + 1) {
                printf("   Each replica receives 2f+1=%d valid COMMITs (commit certificate)\n", 
                      2 * FAULTY_THRESHOLD + 1);
            } else {
                printf("   Not enough valid COMMITs for certificate (%d needed)\n", 
                      2 * FAULTY_THRESHOLD + 1);
            }
            wait_for_key();
            
            // Phase 5: Execute
            printf("5. EXECUTE PHASE:\n");
            int valid_replicas = 0;
            for (int i = 0; i < NUM_REPLICAS; i++) {
                if (!replicas[i].is_faulty) {
                    valid_replicas++;
                    update_kv_store(i, demo_key, demo_value);
                    printf("   Replica %d executes PUT %s=%s\n", i, demo_key, demo_value);
                    replicas[i].seq_num = current_seq;
                } else {
                    printf("   Replica %d (FAULTY) might execute incorrectly or not at all\n", i);
                    
                    // 50% chance for a faulty replica to update incorrectly
                    if (rand() % 2 == 0) {
                        char corrupt_value[MAX_VALUE_SIZE];
                        strcpy(corrupt_value, demo_value);
                        corrupt_value[0] = 'X'; // Simple corruption
                        update_kv_store(i, demo_key, corrupt_value);
                        printf("   Replica %d incorrectly executes PUT %s=%s\n", i, demo_key, corrupt_value);
                    } else {
                        printf("   Replica %d did not execute the operation\n", i);
                    }
                }
            }
            
            printf("\n   Operation complete! %d of %d replicas have consistent state.\n", 
                   valid_replicas, NUM_REPLICAS);
            
            wait_for_key();
        } else if (strcasecmp(cmd, "STATUS") == 0) {
            clear_screen();
            print_header("DETAILED SYSTEM STATUS");
            
            printf("=== REPLICA STATUS ===\n");
            printf("%-10s %-10s %-15s %-10s\n", "REPLICA", "ROLE", "STATUS", "SEQ_NUM");
            printf("----------------------------------------------\n");
            
            for (int i = 0; i < NUM_REPLICAS; i++) {
                printf("%-10d %-10s %-15s %-10d\n", 
                       i, 
                       is_primary(i) ? "PRIMARY" : "BACKUP", 
                       replicas[i].is_faulty ? "FAULTY" : "CORRECT", 
                       replicas[i].seq_num);
            }
            
            printf("\n=== SYSTEM CONFIGURATION ===\n");
            printf("Total replicas:      %d\n", NUM_REPLICAS);
            printf("Fault tolerance (f): %d\n", FAULTY_THRESHOLD);
            printf("Current view:        %d\n", 0);
            printf("Current sequence:    %d\n", current_seq);
            printf("Primary replica:     %d\n", get_primary_for_view(0));
            
            printf("\n=== BFT PROTOCOL PARAMETERS ===\n");
            printf("Protocol:            PBFT (Practical Byzantine Fault Tolerance)\n");
            printf("Required quorum:     2f+1 = %d\n", 2 * FAULTY_THRESHOLD + 1);
            printf("Message pattern:     REQUEST → PRE-PREPARE → PREPARE → COMMIT → EXECUTE\n");
            
            printf("\n=== FAULT STATUS ===\n");
            int faulty_count = 0;
            for (int i = 0; i < NUM_REPLICAS; i++) {
                if (replicas[i].is_faulty) {
                    faulty_count++;
                }
            }
            
            printf("Faulty replicas:     %d of %d\n", faulty_count, NUM_REPLICAS);
            printf("System state:        %s\n", 
                   faulty_count <= FAULTY_THRESHOLD ? "HEALTHY (can tolerate faults)" : "AT RISK (too many faults)");
            
            wait_for_key();
        } else if (strcasecmp(cmd, "MEMORY") == 0) {
            clear_screen();
            print_header("MEMORY USAGE ANALYSIS");
            
            // Calculate memory usage for TinyBFT based on paper
            size_t agreement_size = 37008;  // From the actual implementation
            size_t checkpoint_size = 16420;
            size_t event_size = 16384;
            size_t scratch_size = 4112;
            
            size_t total_static = agreement_size + checkpoint_size + event_size + scratch_size;
            size_t state_machine_size = 16384 + 64; // Application state + management overhead
            size_t total_size = total_static + state_machine_size;
            
            printf("TinyBFT memory regions:\n");
            printf("- Agreement Region:            %7zu bytes (%.1f KB)\n", agreement_size, agreement_size/1024.0);
            printf("- Checkpoint Region:           %7zu bytes (%.1f KB)\n", checkpoint_size, checkpoint_size/1024.0);
            printf("- Event Region:                %7zu bytes (%.1f KB)\n", event_size, event_size/1024.0);
            printf("- Scratch Region:              %7zu bytes (%.1f KB)\n", scratch_size, scratch_size/1024.0);
            printf("Total static memory:           %7zu bytes (%.1f KB)\n", 
                   total_static, total_static / 1024.0);
            printf("Application state:             %7zu bytes (%.1f KB)\n", 
                   state_machine_size, state_machine_size / 1024.0);
            printf("TOTAL MEMORY:                  %7zu bytes (%.1f KB)\n", 
                   total_size, total_size / 1024.0);
            
            // Compare with standard implementation
            size_t std_impl_size = 8600000; // 8.6 MB (from paper)
            float reduction = 100.0 * (1.0 - ((float)total_size / std_impl_size));
            float esp32_usage = 100.0 * total_size / (400.0 * 1024);
            
            printf("\n=== COMPARISON ===\n");
            printf("Memory reduction: %.1f%% vs standard implementation\n", reduction);
            printf("ESP32-C3 RAM usage: %.1f%% of 400KB available RAM\n", esp32_usage);
            
            printf("\n=== KEY INNOVATIONS ===\n");
            printf("1. Static Memory Allocation\n");
            printf("   - All memory allocated at compile time\n");
            printf("   - No dynamic allocation (malloc/free) during operation\n");
            printf("   - Fixed memory footprint regardless of workload\n");
            
            printf("\n2. Four-Region Memory Layout\n");
            printf("   - Agreement Region: Protocol certificates (%.1f KB)\n", agreement_size/1024.0);
            printf("   - Checkpoint Region: Stable checkpoints (%.1f KB)\n", checkpoint_size/1024.0);
            printf("   - Event Region: Messages with varied lifetimes (%.1f KB)\n", event_size/1024.0);
            printf("   - Scratch Region: Temporary processing buffer (%.1f KB)\n", scratch_size/1024.0);
            
            wait_for_key();
        } else if (strcasecmp(cmd, "CLEAR") == 0) {
            // Will clear on next iteration
        } else if (strcasecmp(cmd, "QUIT") == 0 || strcasecmp(cmd, "EXIT") == 0) {
            exit(0);
        } else {
            printf("Unknown command. Type PUT, GET, FAULT, PROCESS, STATUS, MEMORY, CLEAR, or QUIT\n");
            wait_for_key();
        }
    }
}

// Execute PUT command
void execute_put_command(const char* key, const char* value) {
    clear_screen();
    print_header("EXECUTING PUT OPERATION");
    
    printf("Adding key-value pair: '%s' = '%s'\n\n", key, value);
    
    // Increment global sequence number
    current_seq++;
    
    printf("=== PBFT PROTOCOL FLOW ===\n");
    printf("Press a key after each phase to continue...\n\n");
    
    // Simulate request phase
    simulate_request_phase(key, value);
    wait_for_key();
    
    // Simulate pre-prepare phase
    simulate_pre_prepare_phase(key, value);
    wait_for_key();
    
    // Simulate prepare phase
    simulate_prepare_phase();
    wait_for_key();
    
    // Simulate commit phase
    simulate_commit_phase();
    wait_for_key();
    
    // Simulate execute phase
    simulate_execute_phase(key, value);
    wait_for_key();
}

// Execute GET command
void execute_get_command(const char* key) {
    clear_screen();
    print_header("EXECUTING GET OPERATION");
    
    printf("Retrieving values for key: '%s'\n\n", key);
    
    for (int i = 0; i < NUM_REPLICAS; i++) {
        printf("Replica %d: ", i);
        bool found = false;
        
        for (int j = 0; j < MAX_KEYS; j++) {
            if (replicas[i].kv_store[j].used && strcmp(replicas[i].kv_store[j].key, key) == 0) {
                printf("'%s' = '%s'", key, replicas[i].kv_store[j].value);
                found = true;
                break;
            }
        }
        
        if (!found) {
            printf("Key '%s' not found", key);
        }
        printf("\n");
    }
    
    wait_for_key();
}

// Simulate client request phase
void simulate_request_phase(const char* key, const char* value) {
    int primary = get_primary_for_view(0);
    
    printf("1. CLIENT REQUEST PHASE:\n");
    printf("   Client sends request to primary (Replica %d): PUT %s=%s\n", 
           primary, key, value);
}

// Simulate pre-prepare phase
void simulate_pre_prepare_phase(const char* key, const char* value) {
    int primary = get_primary_for_view(0);
    
    printf("2. PRE-PREPARE PHASE:\n");
    printf("   Primary (Replica %d) assigns sequence number %d\n", primary, current_seq);
    printf("   Primary broadcasts PRE-PREPARE to all replicas\n");
    
    // Update primary's sequence number
    replicas[primary].seq_num = current_seq;
}

// Simulate prepare phase
void simulate_prepare_phase() {
    printf("3. PREPARE PHASE:\n");
    
    for (int i = 0; i < NUM_REPLICAS; i++) {
        if (!replicas[i].is_faulty) {
            printf("   Replica %d broadcasts PREPARE message\n", i);
        } else {
            printf("   Replica %d (FAULTY) might send corrupt PREPARE or none at all\n", i);
        }
    }
    
    int valid_prepares = 0;
    for (int i = 0; i < NUM_REPLICAS; i++) {
        if (!replicas[i].is_faulty) {
            valid_prepares++;
        }
    }
    
    printf("\n   Total valid PREPARE messages: %d\n", valid_prepares);
    
    if (valid_prepares >= 2 * FAULTY_THRESHOLD + 1) {
        printf("   Each replica receives 2f+1=%d valid PREPAREs (prepare certificate)\n", 
              2 * FAULTY_THRESHOLD + 1);
    } else {
        printf("   Not enough valid PREPAREs for certificate (%d needed)\n", 
              2 * FAULTY_THRESHOLD + 1);
    }
}

// Simulate commit phase
void simulate_commit_phase() {
    printf("4. COMMIT PHASE:\n");
    
    for (int i = 0; i < NUM_REPLICAS; i++) {
        if (!replicas[i].is_faulty) {
            printf("   Replica %d broadcasts COMMIT message\n", i);
        } else {
            printf("   Replica %d (FAULTY) might send corrupt COMMIT or none at all\n", i);
        }
    }
    
    int valid_commits = 0;
    for (int i = 0; i < NUM_REPLICAS; i++) {
        if (!replicas[i].is_faulty) {
            valid_commits++;
        }
    }
    
    printf("\n   Total valid COMMIT messages: %d\n", valid_commits);
    
    if (valid_commits >= 2 * FAULTY_THRESHOLD + 1) {
        printf("   Each replica receives 2f+1=%d valid COMMITs (commit certificate)\n", 
              2 * FAULTY_THRESHOLD + 1);
    } else {
        printf("   Not enough valid COMMITs for certificate (%d needed)\n", 
              2 * FAULTY_THRESHOLD + 1);
    }
}

// Simulate execute phase
void simulate_execute_phase(const char* key, const char* value) {
    printf("5. EXECUTE PHASE:\n");
    
    int valid_replicas = 0;
    for (int i = 0; i < NUM_REPLICAS; i++) {
        if (!replicas[i].is_faulty) {
            valid_replicas++;
            update_kv_store(i, key, value);
            printf("   Replica %d executes PUT %s=%s\n", i, key, value);
            replicas[i].seq_num = current_seq;
        } else {
            printf("   Replica %d (FAULTY) might execute incorrectly or not at all\n", i);
            
            // 50% chance for a faulty replica to update incorrectly
            if (rand() % 2 == 0) {
                char corrupt_value[MAX_VALUE_SIZE];
                strcpy(corrupt_value, value);
                corrupt_value[0] = 'X'; // Simple corruption
                update_kv_store(i, key, corrupt_value);
                printf("   Replica %d incorrectly executes PUT %s=%s\n", i, key, corrupt_value);
            } else {
                printf("   Replica %d did not execute the operation\n", i);
            }
        }
    }
    
    printf("\n   Operation complete! %d of %d replicas have consistent state.\n", 
           valid_replicas, NUM_REPLICAS);
}

// Update key-value store
void update_kv_store(int replica_id, const char* key, const char* value) {
    if (key[0] == '\0') {
        return;
    }
    
    // Find the key or an empty slot
    int empty_slot = -1;
    for (int i = 0; i < MAX_KEYS; i++) {
        if (replicas[replica_id].kv_store[i].used && 
            strcmp(replicas[replica_id].kv_store[i].key, key) == 0) {
            // Update existing key
            strncpy(replicas[replica_id].kv_store[i].value, value, MAX_VALUE_SIZE - 1);
            replicas[replica_id].kv_store[i].value[MAX_VALUE_SIZE - 1] = '\0';
            return;
        }
        
        if (!replicas[replica_id].kv_store[i].used && empty_slot < 0) {
            empty_slot = i;
        }
    }
    
    // Add new key if slot available
    if (empty_slot >= 0) {
        strncpy(replicas[replica_id].kv_store[empty_slot].key, key, MAX_KEY_SIZE - 1);
        replicas[replica_id].kv_store[empty_slot].key[MAX_KEY_SIZE - 1] = '\0';
        
        strncpy(replicas[replica_id].kv_store[empty_slot].value, value, MAX_VALUE_SIZE - 1);
        replicas[replica_id].kv_store[empty_slot].value[MAX_VALUE_SIZE - 1] = '\0';
        
        replicas[replica_id].kv_store[empty_slot].used = true;
    }
}

// Display detailed status
void display_status() {
    clear_screen();
    print_header("DETAILED SYSTEM STATUS");
    
    printf("=== REPLICA STATUS ===\n");
    printf("%-10s %-10s %-15s %-10s\n", "REPLICA", "ROLE", "STATUS", "SEQ_NUM");
    printf("----------------------------------------------\n");
    
    for (int i = 0; i < NUM_REPLICAS; i++) {
        printf("%-10d %-10s %-15s %-10d\n", 
               i, 
               is_primary(i) ? "PRIMARY" : "BACKUP", 
               replicas[i].is_faulty ? "FAULTY" : "CORRECT", 
               replicas[i].seq_num);
    }
    
    printf("\n=== SYSTEM CONFIGURATION ===\n");
    printf("Total replicas:      %d\n", NUM_REPLICAS);
    printf("Fault tolerance (f): %d\n", FAULTY_THRESHOLD);
    printf("Current view:        %d\n", 0);
    printf("Current sequence:    %d\n", current_seq);
    printf("Primary replica:     %d\n", get_primary_for_view(0));
    
    printf("\n=== BFT PROTOCOL PARAMETERS ===\n");
    printf("Protocol:            PBFT (Practical Byzantine Fault Tolerance)\n");
    printf("Required quorum:     2f+1 = %d\n", 2 * FAULTY_THRESHOLD + 1);
    printf("Message pattern:     REQUEST → PRE-PREPARE → PREPARE → COMMIT → EXECUTE\n");
    
    printf("\n=== FAULT STATUS ===\n");
    int faulty_count = 0;
    for (int i = 0; i < NUM_REPLICAS; i++) {
        if (replicas[i].is_faulty) {
            faulty_count++;
        }
    }
    
    printf("Faulty replicas:     %d of %d\n", faulty_count, NUM_REPLICAS);
    printf("System state:        %s\n", 
           faulty_count <= FAULTY_THRESHOLD ? "HEALTHY (can tolerate faults)" : "AT RISK (too many faults)");
}

// Display key-value stores
void display_key_value_stores() {
    // Find all unique keys
    char keys[MAX_KEYS][MAX_KEY_SIZE];
    int key_count = 0;
    
    for (int i = 0; i < NUM_REPLICAS; i++) {
        for (int j = 0; j < MAX_KEYS; j++) {
            if (replicas[i].kv_store[j].used) {
                bool found = false;
                for (int k = 0; k < key_count; k++) {
                    if (strcmp(keys[k], replicas[i].kv_store[j].key) == 0) {
                        found = true;
                        break;
                    }
                }
                
                if (!found && key_count < MAX_KEYS) {
                    strncpy(keys[key_count], replicas[i].kv_store[j].key, MAX_KEY_SIZE - 1);
                    keys[key_count][MAX_KEY_SIZE - 1] = '\0';
                    key_count++;
                }
            }
        }
    }
    
    if (key_count == 0) {
        printf("No keys stored yet. Use PUT command to add key-value pairs.\n");
        return;
    }
    
    // Print header
    printf("%-10s ", "KEY");
    for (int i = 0; i < NUM_REPLICAS; i++) {
        printf("REPLICA%-2d    ", i);
    }
    printf("\n");
    printf("----------------------------------------------\n");
    
    // Print values for each key
    for (int k = 0; k < key_count; k++) {
        printf("%-10s ", keys[k]);
        
        for (int i = 0; i < NUM_REPLICAS; i++) {
            bool found = false;
            for (int j = 0; j < MAX_KEYS; j++) {
                if (replicas[i].kv_store[j].used && 
                    strcmp(replicas[i].kv_store[j].key, keys[k]) == 0) {
                    printf("%-12s ", replicas[i].kv_store[j].value);
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                printf("%-12s ", "---");
            }
        }
        printf("\n");
    }
}

// Display memory usage
void display_memory_usage() {
    clear_screen();
    print_header("MEMORY USAGE ANALYSIS");
    
    // Calculate memory usage for TinyBFT based on paper
    size_t agreement_size = 37008;  // From the actual implementation
    size_t checkpoint_size = 16420;
    size_t event_size = 16384;
    size_t scratch_size = 4112;
    
    size_t total_static = agreement_size + checkpoint_size + event_size + scratch_size;
    size_t state_machine_size = 16384 + 64; // Application state + management overhead
    size_t total_size = total_static + state_machine_size;
    
    printf("TinyBFT memory regions:\n");
    printf("- Agreement Region:            %7zu bytes\n", agreement_size);
    printf("- Checkpoint Region:           %7zu bytes\n", checkpoint_size);
    printf("- Event Region:                %7zu bytes\n", event_size);
    printf("- Scratch Region:              %7zu bytes\n", scratch_size);
    printf("Total static memory:           %7zu bytes (%.1f KB)\n", 
           total_static, total_static / 1024.0);
    printf("Application state:             %7zu bytes (%.1f KB)\n", 
           state_machine_size, state_machine_size / 1024.0);
    printf("TOTAL MEMORY:                  %7zu bytes (%.1f KB)\n", 
           total_size, total_size / 1024.0);
    
    // Compare with standard implementation
    size_t std_impl_size = 8600000; // 8.6 MB (from paper)
    float reduction = 100.0 * (1.0 - ((float)total_size / std_impl_size));
    float esp32_usage = 100.0 * total_size / (400.0 * 1024);
    
    printf("\n=== COMPARISON ===\n");
    printf("Memory reduction: %.1f%% vs standard implementation\n", reduction);
    printf("ESP32-C3 RAM usage: %.1f%% of 400KB available RAM\n", esp32_usage);
    
    printf("\n=== KEY INNOVATIONS ===\n");
    printf("1. Static Memory Allocation\n");
    printf("   - All memory allocated at compile time\n");
    printf("   - No dynamic allocation (malloc/free) during operation\n");
    printf("   - Fixed memory footprint regardless of workload\n");
    
    printf("\n2. Four-Region Memory Layout\n");
    printf("   - Agreement Region: Protocol certificates (%.1f KB)\n", agreement_size/1024.0);
    printf("   - Checkpoint Region: Stable checkpoints (%.1f KB)\n", checkpoint_size/1024.0);
    printf("   - Event Region: Messages with varied lifetimes (%.1f KB)\n", event_size/1024.0);
    printf("   - Scratch Region: Temporary processing buffer (%.1f KB)\n", scratch_size/1024.0);
}

// Clear the screen
void clear_screen() {
    system("cls");
}

// Print a header
void print_header(const char* title) {
    printf("=== %s ===\n", title);
}

// Wait for key press
void wait_for_key() {
    printf("\nPress any key to continue...");
    _getch();
}