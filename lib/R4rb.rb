# loading library: compatible for ruby-1.8.7, ruby-1.9.3, ruby-2.0.0
version=`Rscript -e 'cat(R.version$major,R.version$minor,sep=".")'`
version=version.split(".")[0...2].join(".")
#puts version
found=nil
[".bundle",".so"].each do |ext|
	["","../ext/R4rb"].each do |path| #sometimes in lib sometimes in ext/R4rb/
		lib=File.join(File.dirname(__FILE__),path,'R4rb.'+version+ext)
		if File.exists? lib
			require lib
			found=true
			break
		end
	end
	break if found
end
require 'R4rb.so' unless found

# loading ruby files
require 'R4rb/R2rb_init'
require 'R4rb/R2rb_eval'
require 'R4rb/robj'
require 'R4rb/Rserve'
require 'R4rb/converter'
require 'R4rb/R4rb'
