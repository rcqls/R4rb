Gem::Specification.new do |s|
    s.platform = Gem::Platform::CURRENT
    s.summary = "R for ruby"
    s.name = "R4rb"
    s.version = '1.0.0'
    s.requirements << 'none'
    s.require_paths = ["lib"]
    s.files = Dir['lib/**/*.rb'] + Dir['lib/*.so']
    s.required_ruby_version = '>= 1.8.0'
    s.description = <<-EOF
  R is embedded in ruby with some communication support .
  EOF
    s.author = "CQLS"
    s.email= "rdrouilh@gmail.com"
    s.homepage = "http://cqls.upmf-grenoble.fr"
    s.rubyforge_project = nil
    s.has_rdoc = false
end
