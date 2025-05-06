#ifndef TINYBFT_MEMORY_LAYOUT_H
#define TINYBFT_MEMORY_LAYOUT_H

#include <stdint.h>
#include <stdbool.h>

// Configuration settings (should be customizable via build system)
#ifndef TINYBFT_MAX_REPLICAS
#define TINYBFT_MAX_REPLICAS 4  // Default to f=1 (requires 3f+1=4 replicas)
#endif

#ifndef TINYBFT_MAX_FAULTY
#define TINYBFT_MAX_FAULTY ((TINYBFT_MAX_REPLICAS - 1) / 3)  // Maximum faulty replicas (f)
#endif

#ifndef TINYBFT_WINDOW_SIZE
#define TINYBFT_WINDOW_SIZE 4   // Size of the agreement window (W)
#endif

#ifndef TINYBFT_CHECKPOINT_INTERVAL
#define TINYBFT_CHECKPOINT_INTERVAL 2  // Checkpoint interval (K)
#endif

#ifndef TINYBFT_MAX_CLIENTS
#define TINYBFT_MAX_CLIENTS 4   // Maximum number of clients
#endif

#ifndef TINYBFT_MAX_MSG_SIZE
#define TINYBFT_MAX_MSG_SIZE 1024   // Maximum message size in bytes
#endif

#ifndef TINYBFT_MAX_STATE_SIZE
#define TINYBFT_MAX_STATE_SIZE 16384  // Maximum application state size (16 KiB)
#endif

#ifndef TINYBFT_BLOCK_SIZE
#define TINYBFT_BLOCK_SIZE 1024   // Size of state blocks (1 KiB)
#endif

// Memory region types
typedef enum {
    MEMORY_REGION_AGREEMENT = 0,
    MEMORY_REGION_CHECKPOINT,
    MEMORY_REGION_EVENT,
    MEMORY_REGION_SCRATCH,
    MEMORY_REGION_COUNT
} tinybft_memory_region_t;

// Message types
typedef enum {
    MSG_TYPE_REQUEST = 0,
    MSG_TYPE_REPLY,
    MSG_TYPE_PRE_PREPARE,
    MSG_TYPE_PREPARE,
    MSG_TYPE_COMMIT,
    MSG_TYPE_CHECKPOINT,
    MSG_TYPE_VIEW_CHANGE,
    MSG_TYPE_NEW_VIEW,
    MSG_TYPE_STATE_TRANSFER_REQ,
    MSG_TYPE_STATE_TRANSFER_RESP
} tinybft_msg_type_t;

// Message structure (header)
typedef struct {
    tinybft_msg_type_t type;
    uint32_t sender_id;
    uint32_t receiver_id;  // Added receiver_id for message routing
    uint32_t view;
    uint32_t seq_num;
    uint32_t data_len;
    // Message data follows this header (variable size)
} tinybft_msg_header_t;

// Certificate structures
typedef struct {
    uint32_t view;
    uint32_t seq_num;
    bool valid;
    uint8_t pre_prepare[TINYBFT_MAX_MSG_SIZE];
    uint8_t prepares[TINYBFT_MAX_REPLICAS][TINYBFT_MAX_MSG_SIZE];
    uint32_t prepare_count;
} tinybft_prepare_certificate_t;

typedef struct {
    uint32_t view;
    uint32_t seq_num;
    bool valid;
    uint8_t commits[TINYBFT_MAX_REPLICAS][TINYBFT_MAX_MSG_SIZE];
    uint32_t commit_count;
} tinybft_commit_certificate_t;

typedef struct {
    uint32_t seq_num;
    bool valid;
    uint8_t checkpoints[TINYBFT_MAX_REPLICAS][TINYBFT_MAX_MSG_SIZE];
    uint32_t checkpoint_count;
} tinybft_checkpoint_certificate_t;

// Agreement slot for window
typedef struct {
    uint32_t seq_num;
    tinybft_prepare_certificate_t prepare_cert;
    tinybft_commit_certificate_t commit_cert;
} tinybft_agreement_slot_t;

// Memory layout for each region
typedef struct {
    tinybft_agreement_slot_t slots[TINYBFT_WINDOW_SIZE];
} tinybft_agreement_region_t;

typedef struct {
    tinybft_checkpoint_certificate_t certificates[TINYBFT_WINDOW_SIZE / TINYBFT_CHECKPOINT_INTERVAL + 1];
    uint8_t checkpoint_msgs[TINYBFT_MAX_REPLICAS][TINYBFT_MAX_MSG_SIZE];
} tinybft_checkpoint_region_t;

typedef struct {
    uint8_t client_requests[TINYBFT_MAX_CLIENTS][TINYBFT_MAX_MSG_SIZE];
    uint8_t client_replies[TINYBFT_MAX_CLIENTS][TINYBFT_MAX_MSG_SIZE];
    uint8_t view_change_msgs[TINYBFT_MAX_REPLICAS][TINYBFT_MAX_MSG_SIZE];
    uint8_t new_view_msgs[TINYBFT_MAX_REPLICAS][TINYBFT_MAX_MSG_SIZE];
} tinybft_event_region_t;

typedef struct {
    uint8_t buffers[TINYBFT_MAX_REPLICAS][TINYBFT_MAX_MSG_SIZE];
    uint32_t buffer_used[TINYBFT_MAX_REPLICAS];
} tinybft_scratch_region_t;

// Partition tree for state management
typedef struct partition_node {
    uint32_t block_index;
    uint32_t block_count;
    uint64_t version;
    uint8_t hash[32];  // SHA-256 hash
    struct partition_node* children;
} tinybft_partition_node_t;

// Memory layout initialization and management functions
void tinybft_memory_init(void);
void tinybft_memory_set_nvm(bool use_nvm);
void* tinybft_get_region(tinybft_memory_region_t region);
void* tinybft_alloc_from_scratch(uint32_t size);
void tinybft_move_to_region(tinybft_memory_region_t dst_region, void* scratch_ptr, uint32_t size);
tinybft_agreement_slot_t* tinybft_find_agreement_slot(uint32_t seq_num);
tinybft_agreement_slot_t* tinybft_init_agreement_slot(uint32_t seq_num);
tinybft_checkpoint_certificate_t* tinybft_find_checkpoint_cert(uint32_t seq_num);

#endif // TINYBFT_MEMORY_LAYOUT_H