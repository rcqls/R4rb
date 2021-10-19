# encoding: utf-8
require "mkmf"
require 'fileutils' #if RUBY_VERSION < "1.9"

# TODO: specify a file R4rb.conf to provide multiple R systems location
# in lib/R4rb.rb read this file to offer the several choices

def find_installed_R

  if RUBY_PLATFORM=~/mingw/ or RUBY_PLATFORM=~/msys/
    $prefix=`R RHOME`.gsub("\\","/")
    $prefix_include=$prefix+"/include"
    $prefix_lib=nil
    (RUBY_PLATFORM=~/64/ ? ["","x64"] : ["","i386"]).each do |arch|
        $prefix_lib=File.join($prefix,"bin",arch) if File.exists? File.join($prefix,"bin",arch,"R.dll")
        #$versions=[arch]
        break if $prefix_lib
    end
  elsif RUBY_PLATFORM=~/darwin/
    
    # versions="/Library/Frameworks/R.framework/Versions"
    # if File.directory? versions  
    #     $prefix=Dir[versions+"/*/Resources"].select{|e| e.split("/")[-2]!="Current"}
    #     $prefix_include=$prefix.map{|e| e+"/include"}
    #     $prefix_lib=$prefix.map{|e| e+"/lib"}
    #     $versions=$prefix.map{|e| e.split("/")[-2]}
    # else
        $prefix=`R RHOME`.strip
        $prefix_include=$prefix+"/include"
        $prefix_lib=$prefix+"/lib"
    # end
    
  else
    stddirs=["/usr/local/lib/R","/usr/lib/R","/usr/share/R","/usr/include/R","/usr/lib64/R"]
    stddirs.unshift `R RHOME`.strip.split("\n").select{|l| l=~/^\//}[0] unless `R RHOME`.empty?
    usrdirs = []
    ARGV.each do |arg|
        if arg =~ /--with-R/
            option, value = arg.split('=')
            usrdirs = [ value ] + usrdirs
        end
    end
    dirs = usrdirs + stddirs
    dirs.uniq! # remove duplicates 

    $prefix,$prefix_include,$prefix_lib=nil,nil,nil

    dirs.each do |dir|
        p dir
        if !$prefix and FileTest.exists?(dir)
            $prefix = dir[0..-3]
        end
    
        if !$prefix_include and FileTest.exists?(dir+"/include/R.h")
            $prefix_include=dir+"/include"
        end

        if !$prefix_include and FileTest.exists?(dir+"/R.h")
            $prefix_include=dir
        end
    
        if !$prefix_lib and FileTest.exists?(dir+"/lib/libR.so")
            $prefix_lib=dir+"/lib"
        end 
    end
    raise RuntimeError, "couldn't find R Home : R seems to be uninstalled!!" unless $prefix
    raise RuntimeError, "couldn't find R.h!!" unless $prefix_include
    raise RuntimeError, "couldn't find libR.so!!" unless $prefix_lib
    #return multi
  end  

end


def r4rb_makefile(inc,lib,version=nil) 
    $CFLAGS = "-I"+inc+" -I."
    $LDFLAGS = "-L"+lib if lib
    $libs = "-lR"
 
    header = nil

    rb4r_name="R4rb"+((version and version!="orig") ? "."+version : "" )
    $objs = [rb4r_name+".o"]

    dir_config("R4rb")
    create_makefile(rb4r_name)
    File.rename("Makefile", "Makefile-#{version}")
end

## R is installed?
find_installed_R

$versions=["orig"] unless $versions
$prefix,$prefix_include,$prefix_lib=[$prefix],[$prefix_include],[$prefix_lib] unless $prefix.is_a? Array

modules = ""

p $versions

File.unlink("Makefile") if (FileTest.exist? "Makefile")
$versions.each_with_index {|version,i| 
    File.unlink("Makefile-#{version}") if (FileTest.exist? "Makefile-#{version}")
    r4rb_makefile($prefix_include[i],$prefix_lib[i],version)
    rb4r_name="R4rb"+((version and version!="orig") ? "."+version : "" )
    modules += " #{rb4r_name}.#{CONFIG['DLEXT']}"
    FileUtils.cp "R4rb.c", "#{rb4r_name}.c" if "R4rb.c" != "#{rb4r_name}.c"
}

open("Makefile", "w") {|f|
    v = $nmake ? '{$(srcdir)}' : ''
    f << "SHELL = /bin/sh" + "\n"
    f << "srcdir = #{$srcdir}" + "\n"
    f << "VPATH = $(srcdir)" + "\n"

    f << "all: #{modules}" + "\n\n"

    $versions.each do |version|
        rb4r_name="R4rb"+((version and version!="orig") ? "."+version : "" )
        f <<  "#{rb4r_name}.#{CONFIG['DLEXT']}: #{v}#{rb4r_name}.c" + "\n"
        f << "\t@echo Now Making R4rb-#{version} extend module" + "\n"
        f << "\t@$(MAKE) -f Makefile-#{version}" + "\n\n"
    end

    ["clean","distclean","install","site-install"].each do |task|
        f << "#{task}:"
        f << " "+modules if ["install","site-install"].include? task
        f << "\n"
        $versions.each do |version|
            f << "\t@$(MAKE) -f Makefile-#{version} #{task}" + "\n"
        end
        f << "\n"
    end
   
} 
