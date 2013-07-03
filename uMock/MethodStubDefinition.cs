using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace uMock
{
    public class MethodMockDefinition : IMockDefinition
    {
        private Func<Object> methodToMock;
        private Func<Object> methodThatMocks;

        internal MethodMockDefinition(Func<Object> methodInfo)
        {
            methodToMock = methodInfo;
        }


        public void With(Func<Object> mockingMethod)
        {
            methodThatMocks = mockingMethod;
        }
    }
}
