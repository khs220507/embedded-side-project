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

#define STACK_CAPACITY 4U

typedef struct
{
    int data[STACK_CAPACITY];
    size_t top;
} Stack;

typedef enum
{
    STACK_OK,
    STACK_EMPTY,
    STACK_FULL
} StackResult;

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

static void stack_reset(Stack *stack)
{
    stack->top = 0U;
}

static StackResult stack_push(Stack *stack, int value)
{
    if (stack->top >= STACK_CAPACITY)
    {
        return STACK_FULL;
    }

    stack->data[stack->top] = value;
    stack->top++;
    return STACK_OK;
}

static StackResult stack_pop(Stack *stack, int *value)
{
    if (stack->top == 0U)
    {
        return STACK_EMPTY;
    }

    stack->top--;
    *value = stack->data[stack->top];
    return STACK_OK;
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

static bool test_stack_lifo(void)
{
    Stack stack;
    int value = 0;

    stack_reset(&stack);

    return expect(stack_push(&stack, 10) == STACK_OK, "stack accepts first value") &&
           expect(stack_push(&stack, 20) == STACK_OK, "stack accepts second value") &&
           expect(stack_pop(&stack, &value) == STACK_OK && value == 20, "stack pops last value first") &&
           expect(stack_pop(&stack, &value) == STACK_OK && value == 10, "stack pops first value second") &&
           expect(stack_pop(&stack, &value) == STACK_EMPTY, "empty stack is rejected");
}

int main(void)
{
    const bool tests_passed = test_command_completion() &&
                              test_buffer_boundary() &&
                              test_stack_lifo();

    if (!tests_passed)
    {
        return 1;
    }

    printf("All command buffer tests passed.\n");
    return 0;
}
