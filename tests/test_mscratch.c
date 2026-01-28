/*******************************************************************************
 *
 * Test the mscratch register for correct read, write, and atomic behavior.
 *
 * mscratch (0x340) - Machine Scratch Register - MRW
 * A 32-bit read/write register dedicated for machine mode use.
 *
 * Test Coverage:
 * 1. Stress Test: All zeros / all ones (power & fan-out check)
 * 2. Precision Test: Walking ones (short/bridging check)
 * 3. Precision Test: Walking zeros (stuck-at check)
 * 4. Logic Test: Atomic read/write swap (csrrw verification)
 * 5. Logic Test: Atomic bit manipulation (csrrs/csrrc verification)
 *
 ******************************************************************************/

#include <stdio.h>

/* CSR access macros for mscratch register (0x340) */
#define WRITE_CSR(reg, val) asm volatile("csrw " #reg ", %0" ::"r"((unsigned int)(val)))
#define READ_CSR(reg, val) asm volatile("csrr %0, " #reg : "=r"(val))
#define SWAP_CSR(reg, out, in) asm volatile("csrrw %0, " #reg ", %1" : "=r"(out) : "r"((unsigned int)(in)))
#define SET_CSR_BITS(reg, out, mask) asm volatile("csrrs %0, " #reg ", %1" : "=r"(out) : "r"((unsigned int)(mask)))
#define CLEAR_CSR_BITS(reg, out, mask) asm volatile("csrrc %0, " #reg ", %1" : "=r"(out) : "r"((unsigned int)(mask)))


int main()
{
    unsigned int write_val, read_back, old_val;
    int error_count = 0;

    printf("\n===== mscratch Register Test Suite =====\n");
    printf("Testing all 32 bits of mscratch CSR (0x340)\n\n");

    /*=========================================================================
     * TEST 1: Stress Test - All zeros / All ones
     * Purpose: Verify basic write/read and check power/fan-out behavior
     *========================================================================*/
    printf("Test 1: Stress checks (all zeros/ones)...\n");

    /* Test all zeros */
    WRITE_CSR(mscratch, 0x00000000);
    READ_CSR(mscratch, read_back);
    if (read_back != 0x00000000) {
        error_count++;
        printf("\tERROR: Failed all-zeros test! Wrote 0x00000000, Read 0x%08X\n", read_back);
    } else {
        printf("\tPASS: All-zeros test (0x00000000)\n");
    }

    /* Test all ones */
    WRITE_CSR(mscratch, 0xFFFFFFFF);
    READ_CSR(mscratch, read_back);
    if (read_back != 0xFFFFFFFF) {
        error_count++;
        printf("\tERROR: Failed all-ones test! Wrote 0xFFFFFFFF, Read 0x%08X\n", read_back);
    } else {
        printf("\tPASS: All-ones test (0xFFFFFFFF)\n");
    }

    /*=========================================================================
     * TEST 2: Walking Ones
     * Purpose: Detect shorts between adjacent bits and bridging faults
     *========================================================================*/
    printf("\nTest 2: Walking ones (bit isolation check)...\n");
    for (int i = 0; i < 32; i++) {
        write_val = (1U << i);
        WRITE_CSR(mscratch, write_val);
        READ_CSR(mscratch, read_back);

        if (read_back != write_val) {
            error_count++;
            printf("\tERROR: Bit %2d failed! Wrote 0x%08X, Read 0x%08X\n",
                   i, write_val, read_back);
        }
    }
    printf("\tCompleted walking ones test for bits 0-31\n");

    /*=========================================================================
     * TEST 3: Walking Zeros
     * Purpose: Detect stuck-at-one faults for each bit position
     *========================================================================*/
    printf("\nTest 3: Walking zeros (stuck-at-one check)...\n");
    for (int i = 0; i < 32; i++) {
        write_val = ~(1U << i);
        WRITE_CSR(mscratch, write_val);
        READ_CSR(mscratch, read_back);

        if (read_back != write_val) {
            error_count++;
            printf("\tERROR: Bit %2d stuck! Wrote 0x%08X, Read 0x%08X\n",
                   i, write_val, read_back);
        }
    }
    printf("\tCompleted walking zeros test for bits 0-31\n");

    /*=========================================================================
     * TEST 4: Atomic Swap (CSRRW)
     * Purpose: Verify atomic read-modify-write behavior
     *========================================================================*/
    printf("\nTest 4: Atomic swap logic (csrrw)...\n");

    unsigned int pattern_a = 0xAAAAAAAA;  /* Alternating 10101010... */
    unsigned int pattern_b = 0x55555555;  /* Alternating 01010101... */

    /* Write initial pattern */
    WRITE_CSR(mscratch, pattern_a);

    /* Atomically swap with new pattern, capturing old value */
    SWAP_CSR(mscratch, old_val, pattern_b);

    /* Verify we got the OLD value back correctly */
    if (old_val != pattern_a) {
        error_count++;
        printf("\tERROR: Swap read failed! Expected 0x%08X, Got 0x%08X\n",
               pattern_a, old_val);
    } else {
        printf("\tPASS: CSRRW returned correct old value (0x%08X)\n", old_val);
    }

    /* Verify the NEW value was written */
    READ_CSR(mscratch, read_back);
    if (read_back != pattern_b) {
        error_count++;
        printf("\tERROR: Swap write failed! Expected 0x%08X, Got 0x%08X\n",
               pattern_b, read_back);
    } else {
        printf("\tPASS: CSRRW wrote correct new value (0x%08X)\n", read_back);
    }

    /*=========================================================================
     * TEST 5: Atomic Bit Manipulation (CSRRS/CSRRC)
     * Purpose: Verify atomic set and clear bit operations
     *========================================================================*/
    printf("\nTest 5: Atomic bit manipulation (csrrs/csrrc)...\n");

    /* Initialize to zero */
    WRITE_CSR(mscratch, 0);

    /* Test atomic set (csrrs) - set lower nibble */
    unsigned int set_mask = 0x0000000F;
    SET_CSR_BITS(mscratch, old_val, set_mask);

    if (old_val != 0) {
        error_count++;
        printf("\tERROR: CSRRS returned wrong old value! Expected 0x0, Got 0x%08X\n", old_val);
    } else {
        printf("\tPASS: CSRRS returned correct old value (0x00000000)\n");
    }

    READ_CSR(mscratch, read_back);
    if (read_back != 0x0000000F) {
        error_count++;
        printf("\tERROR: CSRRS failed to set bits! Expected 0x0000000F, Got 0x%08X\n", read_back);
    } else {
        printf("\tPASS: CSRRS set bits correctly (0x0000000F)\n");
    }

    /* Test atomic clear (csrrc) - clear bit 0 */
    unsigned int clear_mask = 0x00000001;
    CLEAR_CSR_BITS(mscratch, old_val, clear_mask);

    if (old_val != 0x0000000F) {
        error_count++;
        printf("\tERROR: CSRRC returned wrong old value! Expected 0xF, Got 0x%08X\n", old_val);
    } else {
        printf("\tPASS: CSRRC returned correct old value (0x0000000F)\n");
    }

    READ_CSR(mscratch, read_back);
    if (read_back != 0x0000000E) {
        error_count++;
        printf("\tERROR: CSRRC failed to clear bit! Expected 0x0000000E, Got 0x%08X\n", read_back);
    } else {
        printf("\tPASS: CSRRC cleared bit correctly (0x0000000E)\n");
    }

    /*=========================================================================
     * Cleanup and Summary
     *========================================================================*/
    WRITE_CSR(mscratch, 0);

    printf("\n===== Test Summary =====\n");
    printf("Total errors: %d\n", error_count);

    if (error_count == 0) {
        printf("RESULT: ALL TESTS PASSED - Full 32-bit read/write access verified!\n");
    } else {
        printf("RESULT: TESTS FAILED - See errors above\n");
    }

    printf("========================\n\n");

    return error_count;
}
