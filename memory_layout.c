#include "memory_layout.h"
#include <string.h>

// Memory regions
static tinybft_agreement_region_t agreement_region;
static tinybft_checkpoint_region_t checkpoint_region;
static tinybft_event_region_t event_region;
static tinybft_scratch_region_t scratch_region;

// Flag to indicate if the regions are in non-volatile memory
static bool using_nvm = false;

// Initialize the memory regions
void tinybft_memory_init(void) {
    // Zero out all memory regions
    memset(&agreement_region, 0, sizeof(agreement_region));
    memset(&checkpoint_region, 0, sizeof(checkpoint_region));
    memset(&event_region, 0, sizeof(event_region));
    memset(&scratch_region, 0, sizeof(scratch_region));
}

// Set non-volatile memory usage
void tinybft_memory_set_nvm(bool use_nvm) {
    using_nvm = use_nvm;
    
    // In a real implementation, if using_nvm is true, we would:
    // 1. Map agreement_region to non-volatile memory
    // 2. Map checkpoint_region to non-volatile memory
    // 3. Map event_region to non-volatile memory
    // While keeping scratch_region in volatile memory for faster access
}

// Get a pointer to the specified memory region
void* tinybft_get_region(tinybft_memory_region_t region) {
    switch (region) {
        case MEMORY_REGION_AGREEMENT:
            return &agreement_region;
        case MEMORY_REGION_CHECKPOINT:
            return &checkpoint_region;
        case MEMORY_REGION_EVENT:
            return &event_region;
        case MEMORY_REGION_SCRATCH:
            return &scratch_region;
        default:
            return NULL;
    }
}

// Allocate space from the scratch region
void* tinybft_alloc_from_scratch(uint32_t size) {
    if (size > TINYBFT_MAX_MSG_SIZE) {
        return NULL;  // Too large
    }
    
    // Find an empty buffer
    for (uint32_t i = 0; i < TINYBFT_MAX_REPLICAS; i++) {
        if (scratch_region.buffer_used[i] == 0) {
            scratch_region.buffer_used[i] = size;
            return scratch_region.buffers[i];
        }
    }
    
    return NULL;  // No free buffers
}

// Move data from scratch to another region
void tinybft_move_to_region(tinybft_memory_region_t dst_region, void* scratch_ptr, uint32_t size) {
    // First, identify which scratch buffer this is
    uint32_t buffer_idx = 0;
    bool found = false;
    
    for (uint32_t i = 0; i < TINYBFT_MAX_REPLICAS; i++) {
        if (scratch_region.buffers[i] == scratch_ptr) {
            buffer_idx = i;
            found = true;
            break;
        }
    }
    
    if (!found || scratch_region.buffer_used[buffer_idx] == 0) {
        return;  // Invalid scratch pointer
    }
    
    // Copy to the appropriate region based on the destination and message type
    // This is a simplified implementation - a real one would determine the 
    // exact location within the region based on message type and content
    if (dst_region == MEMORY_REGION_AGREEMENT) {
        // Handle copying to agreement region - would need to examine message content
        // to determine the exact location in the agreement slots
    } else if (dst_region == MEMORY_REGION_CHECKPOINT) {
        // Handle copying to checkpoint region
    } else if (dst_region == MEMORY_REGION_EVENT) {
        // Handle copying to event region
    }
    
    // Reset the scratch buffer usage
    scratch_region.buffer_used[buffer_idx] = 0;
}

// Find an agreement slot by sequence number
tinybft_agreement_slot_t* tinybft_find_agreement_slot(uint32_t seq_num) {
    for (uint32_t i = 0; i < TINYBFT_WINDOW_SIZE; i++) {
        if (agreement_region.slots[i].seq_num == seq_num) {
            return &agreement_region.slots[i];
        }
    }
    return NULL;
}

// Initialize an agreement slot for a new sequence number
tinybft_agreement_slot_t* tinybft_init_agreement_slot(uint32_t seq_num) {
    // Find the oldest slot (to reuse)
    uint32_t oldest_idx = 0;
    uint32_t oldest_seq = UINT32_MAX;
    
    for (uint32_t i = 0; i < TINYBFT_WINDOW_SIZE; i++) {
        if (agreement_region.slots[i].seq_num < oldest_seq) {
            oldest_seq = agreement_region.slots[i].seq_num;
            oldest_idx = i;
        }
    }
    
    // Reset and initialize the slot
    tinybft_agreement_slot_t* slot = &agreement_region.slots[oldest_idx];
    memset(slot, 0, sizeof(tinybft_agreement_slot_t));
    slot->seq_num = seq_num;
    
    return slot;
}

// Find or initialize checkpoint certificate for sequence number
tinybft_checkpoint_certificate_t* tinybft_find_checkpoint_cert(uint32_t seq_num) {
    uint32_t num_certs = TINYBFT_WINDOW_SIZE / TINYBFT_CHECKPOINT_INTERVAL + 1;
    
    for (uint32_t i = 0; i < num_certs; i++) {
        if (checkpoint_region.certificates[i].seq_num == seq_num) {
            return &checkpoint_region.certificates[i];
        }
    }
    
    // If not found, initialize one
    uint32_t oldest_idx = 0;
    uint32_t oldest_seq = UINT32_MAX;
    
    for (uint32_t i = 0; i < num_certs; i++) {
        if (checkpoint_region.certificates[i].seq_num < oldest_seq && 
            !checkpoint_region.certificates[i].valid) {
            oldest_seq = checkpoint_region.certificates[i].seq_num;
            oldest_idx = i;
        }
    }
    
    tinybft_checkpoint_certificate_t* cert = &checkpoint_region.certificates[oldest_idx];
    memset(cert, 0, sizeof(tinybft_checkpoint_certificate_t));
    cert->seq_num = seq_num;
    
    return cert;
}