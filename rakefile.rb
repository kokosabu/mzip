PROJECT_CEEDLING_ROOT = "vendor/ceedling"
load "#{PROJECT_CEEDLING_ROOT}/lib/ceedling.rb"

C_COMPILER = "gcc"

Ceedling.load_project

# task :default => %w[ test:all release ]
task :all => ["clean", "build"]
task :default => "build"

task :build => "mzip"

task :clean do
  puts "---- clean ----"
  sh "rm src/*.o"
end

file "mzip" => ["src/main.o", "src/zip.o", "src/crc32.o", "src/deflate.o"] do |t|
  puts "---- link ----"
  sh "#{C_COMPILER} -o #{t.name} #{t.prerequisites.join(' ')}"
end

rule '.o' => '.c' do |t|
  sh "#{C_COMPILER} -c #{t.source} -o #{t.name}"
end
