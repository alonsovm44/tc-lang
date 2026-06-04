# Queues and Stacks
Instead of Channels, Tig now uses Queues and Stacks, because they're more intuitive to me and so we can reuse
stacks and queues for general purpose coding patterns.

## Before

```tig
chan i32 x
```

## After

```tig
queue i32 x = {0,1,2,3}
x.push(4)
i32 y = x.pop()

```
This is easier to understand than channels

## async fns using queues

Since async functions can’t return values, you pass a queue as an argument to the async function, and it sends the result into that queue.
The caller then receives the result from the queue.

```tig
// Async function: takes a queue as an argument
async fn void calculateSum: i32 a, i32 b, queue i32 result {
    i32 sum = a + b
    result.push(sum)  // Send the result into the queue
}

fn void main: {
    queue i32 result_q  // Create a queue for the result
    async calculateSum(3, 4, result_q)  // Start async task
    i32 sum = result_q.pop()  // Receive the result from the queue

    printi(sum)  // Prints 7
}
```
### Safett
Add blocking behavior to queues when used in async contexts.
Pros: Retains the blocking semantics of channels.
Cons: More complex implementation.
Example

```tig
queue i32(10) result_q  // Buffered queue with size 10
async calculateSum(3, 4, result_q)
i32 sum = result_q.pop()  // Blocks if queue is empty
```



## Stacks
LIFO stacks can also be used as queues, but they're more intuitive for some use cases.

```tig
stack i32 x = {0,1,2,3}
x.push(4)
i32 y = x.pop()

```
