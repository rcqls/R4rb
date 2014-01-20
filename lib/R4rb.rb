# encoding: UTF-8
Encoding.default_external = Encoding::UTF_8
# loading library: compatible for ruby-1.8.7, ruby-1.9.3, ruby-2.0.0
if RUBY_PLATFORM=~/mingw32/ #because I did not manage to execute the same on mingw32
  #   cmd=begin `Rscript`; rescue; "undefined";end
  #   unless cmd=="undefined"
  #   	cmd="Rscript.exe"
  #   	version=`#{cmd} -e "cat(R.version$major,R.version$minor,sep='.')"`
  #   else 
  #   	if File.exists? (jsonfile=File.join(ENV["HOME"],"dyndoc","studio","win32","dyndoc.json"))
		#    require 'json'
		#    version=JSON.parse(File.read(jsonfile))["Rversion"]
		# end
  #   end
  version=nil
else
	version=`Rscript -e 'cat(R.version$major,R.version$minor,sep=".")'`
	version=version.split(".")[0...2].join(".")
#puts version
end

found=nil
[".bundle",".so"].each do |ext|
	["","../ext/R4rb"].each do |path| #sometimes in lib sometimes in ext/R4rb/
		lib=File.join(File.dirname(__FILE__),path,'R4rb'+(version ? '.'+version+ext : ext ) )
		if File.exists? lib
			puts "#{lib} found"
			require lib
			puts "#{lib} loaded"
			found=true
			break
		end
	end
	break if found
end

# unless found #windows case
# 	["i386","x64"].each do |version|
# 		["","../ext/R4rb"].each do |path|
# 			lib=File.join(File.dirname(__FILE__),path,'R4rb.'+version+'.so')
# 			if File.exists? lib
# 				puts "#{lib} found"
# 				require lib
# 				found=true
# 				break
# 			end
# 		end
# 	end
# end

require 'R4rb.so' unless found

# loading ruby files
require 'R4rb/R2rb_init'
require 'R4rb/R2rb_eval'
require 'R4rb/robj'
require 'R4rb/Rserve'
require 'R4rb/converter'
require 'R4rb/R4rb'
