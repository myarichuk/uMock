using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace uMock
{
    public interface IMockDefinition
    {
        void With(Func<Object> mockingMethod);
    }
}
