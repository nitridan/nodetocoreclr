{
  "targets": [
    {
      "target_name": "ClrLoader",
      "sources": [ "ClrLoader.cc" ],
      "include_dirs": [
      	"<!(node -e \"require('nan')\")"
      ]
    }
  ]
}