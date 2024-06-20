```mermaid
graph TD;
    A-->B;
    A-->C;
    B-->D;
    C-->D;
```
## Sample sequence diagram
Here is a Hello World example.
```uml-sequence-diagram
Title: Hello world example
Bob->Alice: Hello
Alice-->Bob: How are you?
Note left of Bob: Bob thinks
Bob->>Alice: I'm good, thanks! How about you?
Alice-->Bob: I'm doing great, thank you!
```