using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace uMock
{
    public static class Mock       
    {
        public static IMockDefinition Method(Func<Object> methodDelegate)
        {
            return new MethodMockDefinition(methodDelegate);
        }
    }
}
