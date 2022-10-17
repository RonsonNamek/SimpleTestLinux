srcdir := src
bindir := bin
srcfile := $(srcdir)/*.cpp
headfile := $(srcdir)/*.h
outputname := simpletest.clang.out

objects := main.o

output : $(objects)
	clang++ -std=c++20 -stdlib=libc++ $(srcfile) -o $(bindir)/$(outputname)

main.o : $(headfile)


x = foo
y = $(x)-bar   # 使用最终值替换
z := $(x)-bar  # 使用当前值替换
x = xyz

test :
	@echo x=$(x) y=$(y) z:=$(z)

.PHONY : clean
clean :
	-rm $(bindir)/$(outputname)

