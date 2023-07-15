fun square(a: int): int
{
    return a * a
}

fun sayHello(a: bool, b: bool): int
{
    if(a)
    {
        print("Hello World")

        return 23
    }
    else
    {
        if(b)
        {
            print("Hi")

            return 21
        }

        return 45
    }
}

const foo: int = 18

print(square(foo))
print(sayHello(false, true))

if(foo == 17)
{
    print("foo is 17")
}
else
{
    print("foo is not 17")
}
