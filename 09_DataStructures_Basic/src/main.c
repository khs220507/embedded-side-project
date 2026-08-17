#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define COMMAND_BUFFER_CAPACITY 16U

typedef enum
{
    COMMAND_BUFFER_IN_PROGRESS,
    COMMAND_BUFFER_COMPLETE,
    COMMAND_BUFFER_FULL
} CommandBufferResult;

typedef struct
{
    char data[COMMAND_BUFFER_CAPACITY];
    size_t length;
} CommandBuffer;

static void command_buffer_reset(CommandBuffer *buffer)
{
    buffer->length = 0U;
    buffer->data[0] = '\0';
}

static CommandBufferResult command_buffer_append(CommandBuffer *buffer, char character)
{
    if (character == '\n')
    {
        buffer->data[buffer->length] = '\0';
        return COMMAND_BUFFER_COMPLETE;
    }

    if (buffer->length >= (COMMAND_BUFFER_CAPACITY - 1U))
    {
        return COMMAND_BUFFER_FULL;
    }

    buffer->data[buffer->length] = character;
    buffer->length++;
    buffer->data[buffer->length] = '\0';

    return COMMAND_BUFFER_IN_PROGRESS;
}

static bool expect(bool condition, const char *test_name)
{
    if (!condition)
    {
        printf("FAIL: %s\n", test_name);
        return false;
    }

    printf("PASS: %s\n", test_name);
    return true;
}

static bool test_command_completion(void)
{
    const char *command = "led red\n";
    CommandBuffer buffer;
    CommandBufferResult result = COMMAND_BUFFER_IN_PROGRESS;

    command_buffer_reset(&buffer);

    for (size_t index = 0U; command[index] != '\0'; index++)
    {
        result = command_buffer_append(&buffer, command[index]);
    }

    return expect(result == COMMAND_BUFFER_COMPLETE, "newline completes a command") &&
           expect(strcmp(buffer.data, "led red") == 0, "completed command is null terminated") &&
           expect(buffer.length == 7U, "length excludes newline and terminator");
}

static bool test_buffer_boundary(void)
{
    const char *maximum_length_text = "123456789012345";
    CommandBuffer buffer;
    CommandBufferResult result = COMMAND_BUFFER_IN_PROGRESS;

    command_buffer_reset(&buffer);

    for (size_t index = 0U; maximum_length_text[index] != '\0'; index++)
    {
        result = command_buffer_append(&buffer, maximum_length_text[index]);
    }

    return expect(result == COMMAND_BUFFER_IN_PROGRESS, "maximum payload fits") &&
           expect(buffer.length == (COMMAND_BUFFER_CAPACITY - 1U), "payload reserves terminator") &&
           expect(command_buffer_append(&buffer, 'X') == COMMAND_BUFFER_FULL, "overflow is rejected") &&
           expect(strcmp(buffer.data, maximum_length_text) == 0, "overflow keeps existing data");
}

int main(void)
{
    const bool tests_passed = test_command_completion() && test_buffer_boundary();

    if (!tests_passed)
    {
        return 1;
    }

    printf("All command buffer tests passed.\n");
    return 0;
}
