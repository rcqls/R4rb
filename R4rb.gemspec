require 'rubygems'
require 'rubygems/package_task'

PKG_NAME='R4rb'
PKG_VERSION='1.0.0'
PKG_FILES=FileList[
    'Rakefile','R4rb.gemspec',
    'ext/R4rb/*.c',
    'ext/R4rb/extconf.rb',
    'ext/R4rb/MANIFEST',
    'lib/**/*.rb', 
    'test/**/*.rb',
    'script/**/*'
]

spec = Gem::Specification.new do |s|
    s.platform = Gem::Platform::RUBY
    s.summary = "R for ruby"
    s.name = PKG_NAME
    s.version = PKG_VERSION
    s.requirements << 'none'
    s.require_paths = ["lib","ext/R4rb"]
    s.files = PKG_FILES.to_a
    s.extensions = ["ext/R4rb/extconf.rb"]
    s.description = <<-EOF
  R is embedded in ruby with some communication support .
  EOF
    s.author = "CQLS"
    s.email= "rdrouilh@gmail.com"
    s.homepage = "http://cqls.upmf-grenoble.fr"
    s.rubyforge_project = nil
end
