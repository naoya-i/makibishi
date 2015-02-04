# makibishi
Parallel text processor

## Setup

    $ git clone https://github.com/naoya-i/makibishi
    $ cd makibishi
    $ make

## Usage

    $ cat hello.txt
    I am John Henry.
    How are you?
    I love you.
    You love me.
    
    $ bin/makibishi -i hello.txt -c 'echo My process id is {}, and the offset is @; cat > result.{}.txt' -j 2
    My process id is 1, and the offset is 0
    My process id is 2, and the offset is 30

    $ cat result.1.txt
    I am John Henry.
    How are you?
    
    $ cat result.2.txt
    I love you.
    You love me.
