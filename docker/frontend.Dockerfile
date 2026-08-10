# syntax=docker/dockerfile:1
FROM node:20-slim AS build
WORKDIR /src
COPY apps/dashboard/package.json apps/dashboard/package.json
WORKDIR /src/apps/dashboard
RUN npm install
COPY apps/dashboard/ .
RUN npm run build

FROM nginx:1.27-alpine AS runtime
COPY docker/nginx.conf /etc/nginx/conf.d/default.conf
COPY --from=build /src/apps/dashboard/dist /usr/share/nginx/html
EXPOSE 80
HEALTHCHECK --interval=10s --timeout=3s --retries=5 \
    CMD wget -qO- http://127.0.0.1:80/ >/dev/null || exit 1
