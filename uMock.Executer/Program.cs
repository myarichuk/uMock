using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using uMock;

namespace uMock.Executer
{
    class Program
    {
        static void Main(string[] args)
        {
            Mock.Method(() => DateTime.Now).With(() => DateTime.MinValue);
            Console.WriteLine(DateTime.Now.ToString("r"));
            Console.ReadLine();
        }
    }
}
