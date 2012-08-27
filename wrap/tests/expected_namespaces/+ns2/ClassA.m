%-------Constructors-------
%ClassA()
%-------Methods-------
%memberFunction()
%nsArg(ClassB arg)
%nsReturn(double q)
%-------Static Methods-------
%afunction()
%
%For more detailed documentation on GTSAM go to our Doxygen page, which can be found at http://research.cc.gatech.edu/borg/sites/edu.borg/html/index.html
classdef ClassA < handle
  properties
    ptr_ns2ClassA = 0
  end
  methods
    function obj = ClassA(varargin)
      if nargin == 2 && isa(varargin{1}, 'uint64') && varargin{1} == uint64(5139824614673773682)
        my_ptr = varargin{2};
        testNamespaces_wrapper(6, my_ptr);
      elseif nargin == 0
        my_ptr = testNamespaces_wrapper(7);
      else
        error('Arguments do not match any overload of ns2.ClassA constructor');
      end
      obj.ptr_ns2ClassA = my_ptr;
    end

    function delete(obj)
      testNamespaces_wrapper(8, obj.ptr_ns2ClassA);
    end

    function display(obj), obj.print(''); end

    function disp(obj), obj.display; end

    function varargout = memberFunction(this, varargin)
      % memberFunction  memberFunction() : 
      % Doxygen can be found at http://research.cc.gatech.edu/borg/sites/edu.borg/html/index.html
      % 
      % Method Overloads
      % memberFunction()
      if length(varargin) == 0
        varargout{1} = testNamespaces_wrapper(9, this, varargin{:});
      else
        error('Arguments do not match any overload of function ns2.ClassA.memberFunction');
      end
    end

    function varargout = nsArg(this, varargin)
      % nsArg  nsArg(ClassB arg) : 
      % Doxygen can be found at http://research.cc.gatech.edu/borg/sites/edu.borg/html/index.html
      % 
      % Method Overloads
      % nsArg(ClassB arg)
      if length(varargin) == 1 && isa(varargin{1},'ns1.ClassB')
        varargout{1} = testNamespaces_wrapper(10, this, varargin{:});
      else
        error('Arguments do not match any overload of function ns2.ClassA.nsArg');
      end
    end

    function varargout = nsReturn(this, varargin)
      % nsReturn  nsReturn(double q) : 
      % Doxygen can be found at http://research.cc.gatech.edu/borg/sites/edu.borg/html/index.html
      % 
      % Method Overloads
      % nsReturn(double q)
      if length(varargin) == 1 && isa(varargin{1},'double')
        varargout{1} = testNamespaces_wrapper(11, this, varargin{:});
      else
        error('Arguments do not match any overload of function ns2.ClassA.nsReturn');
      end
    end

  end

  methods(Static = true)
    function varargout = Afunction(varargin)
      % afunction  afunction() : 
      % Doxygen can be found at http://research.cc.gatech.edu/borg/sites/edu.borg/html/index.html
      % 
      % Method Overloads
      % afunction()
      if length(varargin) == 0
        varargout{1} = testNamespaces_wrapper(12, varargin{:});
      else
        error('Arguments do not match any overload of function ns2.ClassA.Afunction');
      end
    end

  end
end
