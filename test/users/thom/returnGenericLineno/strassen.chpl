module Strassen
{
    use Random;
    use Time;
    use Rosslyn;
    use Util;

    class StrassenFactory : BenchmarkFactory
    {
        var n : int;

        proc getInstance() : unmanaged Strassen  // Error message line# wrong
        {
            return new unmanaged Strassen("name", false, n);
        }
        proc writeThis(w)
        {
          w <~> "StrassenFactory " <~> n;
        }

    }

    class Strassen : Benchmark
    {
        var n;
        proc writeThis(w)
        {
          w <~> "Strassen " <~> n;
        }
        proc runKernel()
        {
        }
    }
}
