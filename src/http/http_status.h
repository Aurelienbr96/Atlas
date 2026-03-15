//
// Created by Aurélien Brachet on 15/03/2026.
//

#ifndef STATUS_H
#define STATUS_H

struct HttpStatusInfo {
  std::string_view code;
  std::string_view phrase;
};

struct HttpStatus {
  static constexpr HttpStatusInfo OK{"200", "OK"};
  static constexpr HttpStatusInfo NotFound{"404", "Not Found"};
  static constexpr HttpStatusInfo BadRequest{"400", "Bad Request"};
  static constexpr HttpStatusInfo InternalError{"500", "Internal Server Error"};
};

#endif  // STATUS_H
